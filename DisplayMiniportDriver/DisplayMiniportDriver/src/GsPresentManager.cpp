#include "Common.h"
#include "GsPresentManager.hpp"

#include "BlockTransfer.hpp"
#include "HyDevice.hpp"
#include "MemoryAllocator.h"
#include "XStdDefPolyFill.h"
#include <new>

void* GsPresentManager::operator new(const SIZE_T count)  // NOLINT(misc-new-delete-overloads)
{
    return HyAllocate(PagedPool, count, POOL_TAG_PRESENT);
}

GsPresentManager::GsPresentManager(HyMiniportDevice* const deviceContext) noexcept
    : m_DeviceContext(deviceContext)
    , m_PresentThreadShouldStop(0)
    , m_PendingPresentQueue { }
    , m_PresentQueueLock { }
    , m_PresentQueueSemaphore { }
    , m_PresentThreadObject(nullptr)
    , m_QueuedPresentCount(0)
    , m_Started(false)
{ }

NTSTATUS GsPresentManager::Init() noexcept
{
    LOG_DEBUG("GsPresentManager::Init\n");

    //
    // Initialize everything for the present queue.
    //

    // Used by the Cancel-Safe Queue to serialize access.
    KeInitializeSpinLock(&m_PresentQueueLock);
    // Used to wake-up the present thread when there are frames to present.
    KeInitializeSemaphore(&m_PresentQueueSemaphore, 0, MAXLONG);
    // The list of frame to present.
    InitializeListHead(&m_PendingPresentQueue);

    m_PresentThreadShouldStop = 0;
    m_QueuedPresentCount = 0;

    // A temporary handle for the thread. This will be converted into a thread object later.
    HANDLE threadHandle;
    {
        OBJECT_ATTRIBUTES ObjectAttributes;
        InitializeObjectAttributes(&ObjectAttributes, nullptr, OBJ_KERNEL_HANDLE, nullptr, nullptr);

        // Create and start the present thread.s
        const NTSTATUS createThreadStatus = PsCreateSystemThread(
            &threadHandle,     // ThreadHandle
            THREAD_ALL_ACCESS, // DesiredAccess
            &ObjectAttributes, // ObjectAttributes
            nullptr,           // ProcessHandle
            nullptr,           // ClientId
            StaticThreadFunc,  // StartRoutine
            this               // StartContext
        );

        if(!NT_SUCCESS(createThreadStatus))
        {
            LOG_ERROR("GsPresentManager::Init: Failed to create present thread: 0x%08X\n", createThreadStatus);
            return createThreadStatus;
        }
    }

    {
        //
        // Convert the Thread object handle into a pointer to the Thread object
        // itself. Then close the handle.
        //
        const NTSTATUS referenceObjectStatus = ObReferenceObjectByHandle(
            threadHandle, // Handle
            THREAD_ALL_ACCESS, // DesiredAccess
            nullptr, // ObjectType
            KernelMode, // AccessMode
            reinterpret_cast<void**>(&m_PresentThreadObject), // Object
            nullptr // HandleInformation
        );

        if(!NT_SUCCESS(referenceObjectStatus))
        {
            LOG_ERROR("GsPresentManager::Init: Failed to thread handle into thread object: 0x%08X\n", referenceObjectStatus);
            return referenceObjectStatus;
        }
    }

    {
        // Close the temporary thread handle.
        const NTSTATUS closeThreadStatus = ZwClose(threadHandle);

        if(!NT_SUCCESS(closeThreadStatus))
        {
            LOG_WARN("GsPresentManager::Init: Failed to close thread handle: 0x%08X\n", closeThreadStatus);
        }
    }

    m_Started = true;

    return STATUS_SUCCESS;
}

NTSTATUS GsPresentManager::Close() noexcept
{
    LOG_DEBUG("GsPresentManager::Close\n");

    if(!m_Started)
    {
        return STATUS_SUCCESS;
    }

    if(m_PresentThreadObject)
    {
        StopThread();

        (void) KeWaitForSingleObject(
            m_PresentThreadObject, // Object
            Executive,             // WaitReason
            KernelMode,            // WaitMode
            FALSE,                 // Alertable
            nullptr                // Timeout
        );

        (void) ObDereferenceObject(m_PresentThreadObject);

        m_PresentThreadObject = nullptr;
    }

    LOG_DEBUG("GsPresentManager::Close: Thread stopped, clearing %d packets.\n", m_QueuedPresentCount);

    // Free all the queued presents.
    GsPresentData* presentData = nullptr;
    do
    {
        presentData = PopPresent(true);

        if(!presentData)
        {
            break;
        }

        // Unmap unmap and unlock the pages.
        if(presentData->Mdl)
        {
            MmUnlockPages(presentData->Mdl);
            IoFreeMdl(presentData->Mdl);
        }

        // Free the packet now that we're done with it.
        HY_FREE(presentData, POOL_TAG_PRESENT);
    } while(presentData);

    LOG_DEBUG("GsPresentManager::Close: Cleared packets.\n");

    return STATUS_SUCCESS;
}

void GsPresentManager::FlushPipeline() noexcept
{
    LOG_DEBUG("GsPresentManager::FlushPipeline\n");

    // This is probably terrible, but it should clear the pipeline.
    Close();
    Init();

    // // Free all the queued presents.
    // GsPresentData* presentData = nullptr;
    // do
    // {
    //     presentData = PopPresent(true);
    //
    //     if(!presentData)
    //     {
    //         break;
    //     }
    //
    //     // Free the packet now that we're done with it.
    //     HY_FREE(presentData, POOL_TAG_PRESENT);
    // } while(presentData);
    //
    // LOG_DEBUG("GsPresentManager::FlushPipeline: Cleared packets.\n");
}

void GsPresentManager::InsertPresent(
    const void* const pPresentDisplayOnlyVoid,
    const PMDL Mdl,
    const D3DKMDT_VIDPN_PRESENT_PATH_ROTATION Rotation,
    BYTE* const Destination,
    BYTE* const Source,
    const UINT DestBPP,
    const UINT DestPitch,
    const UINT DestWidth,
    const UINT DestHeight
) noexcept
{
    (void) pPresentDisplayOnlyVoid;
    (void) Mdl;
    (void) Rotation;
    (void) Destination;
    (void) Source;
    (void) DestBPP;
    (void) DestPitch;
    (void) DestWidth;
    (void) DestHeight;

    CHECK_IRQL(DISPATCH_LEVEL);
    if constexpr(false)
    {
        LOG_DEBUG("GsPresentManager::InsertPresent\n");
    }


#if DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WIN8
    const IN_CONST_PDXGKARG_PRESENT_DISPLAYONLY pPresentDisplayOnly = reinterpret_cast<IN_CONST_PDXGKARG_PRESENT_DISPLAYONLY>(pPresentDisplayOnlyVoid);

    const SIZE_T movesSize = pPresentDisplayOnly->NumMoves * sizeof(D3DKMT_MOVE_RECT);
    const SIZE_T rectsSize = pPresentDisplayOnly->NumDirtyRects * sizeof(RECT);
    const SIZE_T size = sizeof(GsPresentData) + movesSize + rectsSize;

    void* placement = HyAllocateZeroed(PagedPool, size, POOL_TAG_PRESENT);
    GsPresentData* presentData = ::new(placement) GsPresentData;

    presentData->Mdl = Mdl;
    presentData->NumMoves = pPresentDisplayOnly->NumMoves;
    presentData->NumDirtyRects = pPresentDisplayOnly->NumDirtyRects;
    presentData->pMoves = reinterpret_cast<D3DKMT_MOVE_RECT*>(presentData + 1);
    presentData->pDirtyRect = reinterpret_cast<RECT*>(reinterpret_cast<BYTE*>(presentData + 1) + movesSize);

    if(pPresentDisplayOnly->pMoves)
    {
        (void) memcpy(presentData->pMoves, pPresentDisplayOnly->pMoves, movesSize);
    }

    if(pPresentDisplayOnly->pDirtyRect)
    {
        (void) memcpy(presentData->pDirtyRect, pPresentDisplayOnly->pDirtyRect, rectsSize);
    }

    presentData->Rotation = Rotation;
    presentData->Destination = Destination;
    presentData->Source = Source;
    presentData->DestPitch = DestPitch;
    presentData->PresentPitch = pPresentDisplayOnly->Pitch;
    presentData->DestBPP = DestBPP;
    presentData->DestWidth = DestWidth;
    presentData->DestHeight = DestHeight;
    presentData->SourceId = pPresentDisplayOnly->VidPnSourceId;

    (void) ExInterlockedInsertTailList(&m_PendingPresentQueue, &presentData->ListEntry, &m_PresentQueueLock);
    InterlockedIncrement(&m_QueuedPresentCount);
    WakeThread();
#endif
}

GsPresentData* GsPresentManager::PopPresent(const bool force) noexcept
{
    // Perform an atomic load of the value. 
    const LONG presentCount = InterlockedCompareExchange(&m_QueuedPresentCount, 0, 0);

    // If there aren't any queued presents, then we can return.
    if(presentCount <= 0 && !force)
    {
        // If the stored present count is less than 0, then we'll just set it back to 0.
        if(presentCount < 0)
        {
            LOG_ERROR("GsPresentManager::PopPresent: Queued Present Count was negative: %d\n", presentCount);

            // Safely set the queue to a min value of 0.
            // If the value has already changed, don't touch it.
            (void) InterlockedCompareExchange(&m_QueuedPresentCount, 0, presentCount);
        }

        return nullptr;
    }

    LIST_ENTRY* entry = ExInterlockedRemoveHeadList(&m_PendingPresentQueue, &m_PresentQueueLock);

    // If there are no more entries in the queue, then Remove will return the pointer to the queue itself.
    if(entry == &m_PendingPresentQueue)
    {
        if(!force)
        {
            LOG_ERROR("GsPresentManager::PopPresent: Present Queue was empty, despite a Queued Present Count of %d.\n", presentCount);
        }
        return nullptr;
    }

    // Record that we've popped from the queue.
    InterlockedDecrement(&m_QueuedPresentCount);

    // Convert the list entry to our data.
    return CONTAINING_RECORD(entry, GsPresentData, ListEntry);
}

void GsPresentManager::StopThread() noexcept
{
    // Inform the thread that it should stop.
    (void) InterlockedExchange(&m_PresentThreadShouldStop, 1);
    // Make sure to wake the thread.
    WakeThread();
}

bool GsPresentManager::IsCloseRequested() noexcept
{
    // This checks if m_PresentThreadShouldStop is equal to 1, and if it is, set it to 2.
    // If the value previously stored is not 0, then a close has been requested.
    return InterlockedCompareExchange(&m_PresentThreadShouldStop, 2, 1) != 0;
}

void GsPresentManager::WakeThread() noexcept
{
    if constexpr(false)
    {
        LOG_DEBUG("GsPresentManager::WakeThread\n");
    }

    (void) KeReleaseSemaphore(
        &m_PresentQueueSemaphore, // Semaphore
        1, // Increment, ~~No priority boost~~
        1, // Adjustment
        FALSE // Wait
    );
}

static UINT ColorTable[] = {
    0xFF00FF00,
    0xFF00FFFF,
    0xFFFF0000,
    0xFFFF00FF,
    0xFFFFFF00,
    0xFFFFFFFF,
    0xFF7F00FF,
    0xFF7F7F7F,
    0xFFFF007F,
    0xFFFF7F00,
};
static constexpr UINT ColorTableSize = sizeof(ColorTable) / sizeof(ColorTable[0]);

static UINT CurrentColorTable = 0;

void GsPresentManager::Present(const GsPresentData& data) noexcept
{
    BLT_INFO dstBltInfo;
    dstBltInfo.pBits = data.Destination;
    dstBltInfo.Pitch = data.DestPitch;
    dstBltInfo.BitsPerPel = data.DestBPP;
    dstBltInfo.Offset.x = 0;
    dstBltInfo.Offset.y = 0;
    dstBltInfo.Rotation = data.Rotation;
    dstBltInfo.Width = data.DestWidth;
    dstBltInfo.Height = data.DestHeight;

    BLT_INFO srcBltInfo;
    srcBltInfo.pBits = data.Source;
    srcBltInfo.Pitch = data.PresentPitch;
    srcBltInfo.BitsPerPel = 32;
    srcBltInfo.Offset.x = 0;
    srcBltInfo.Offset.y = 0;
    srcBltInfo.Rotation = D3DKMDT_VPPR_IDENTITY;

    if(data.Rotation == D3DKMDT_VPPR_ROTATE90 || data.Rotation == D3DKMDT_VPPR_ROTATE270)
    {
        srcBltInfo.Width = dstBltInfo.Height;
        srcBltInfo.Height = dstBltInfo.Width;
    }
    else
    {
        srcBltInfo.Width = dstBltInfo.Width;
        srcBltInfo.Height = dstBltInfo.Height;
    }

    LOG_DEBUG("GsPresentManager::Present: Performing Block Transfer. Moves: %d, Dirty: %d.\n", data.NumMoves, data.NumDirtyRects);


    // Copy all the scroll rects from source image to video frame buffer.
    for(UINT i = 0; i < data.NumMoves; ++i)
    {
        BltBits(
            &dstBltInfo,
            &srcBltInfo,
            1, // NumRects
            &data.pMoves[i].DestRect
        );
    }

    // Copy all the dirty rects from source image to video frame buffer.
    for(UINT i = 0; i < data.NumDirtyRects; ++i)
    {
        if constexpr(false)
        {
            // For some reason this function does not work...
            BltBits(
                &dstBltInfo,
                &srcBltInfo,
                1, // NumRects
                &data.pDirtyRect[i]
            );
        }
        else
        {
            BltBits_Dummy(
                &dstBltInfo,
                &srcBltInfo,
                ColorTable[CurrentColorTable % ColorTableSize],
                1, // NumRects
                &data.pDirtyRect[i]
            );
            ++CurrentColorTable;
        }
    }

    // Unmap unmap and unlock the pages.
    if(data.Mdl)
    {
        MmUnlockPages(data.Mdl);
        IoFreeMdl(data.Mdl);
    }

    // Report Present Progress
    m_DeviceContext->ReportPresentProgress(data.SourceId, TRUE);
}

void GsPresentManager::ThreadFunc() noexcept
{
    LOG_DEBUG("GsPresentManager::ThreadFunc\n");

    while(!IsCloseRequested())
    {
        if constexpr(false)
        {
            LOG_DEBUG("GsPresentManager::ThreadFunc: Awaiting\n");
        }

        // Wait for a present request to come in.
        // If one is already in the queue this will return immediately.
        (void) KeWaitForSingleObject(
            &m_PresentQueueSemaphore, // Object
            Executive, // WaitReason
            KernelMode, // WaitMode
            FALSE, // Alertable
            nullptr // Timeout
        );

        if constexpr(false)
        {
            LOG_DEBUG("GsPresentManager::ThreadFunc: Awoken\n");
        }

        // Check if we're being told to close.
        // We can either be woken because there is a packet to process, or because we're exiting.
        if(IsCloseRequested())
        {
            LOG_DEBUG("GsPresentManager::ThreadFunc: Close Requested.\n");

            return;
        }

        GsPresentData* const presentData = PopPresent();

        // It's possible for this to be null if there wasn't actually anything on the queue.
        // That should never happen.
        if(!presentData)
        {
            continue;
        }

        Present(*presentData);

        // Free the packet now that we're done with it.
        HY_FREE(presentData, POOL_TAG_PRESENT);
    }

    LOG_DEBUG("GsPresentManager::ThreadFunc: Close from loop.\n");
}

void GsPresentManager::StaticThreadFunc(HANDLE Context) noexcept
{
    GsPresentManager* presentManager = reinterpret_cast<GsPresentManager*>(Context);
    presentManager->ThreadFunc();
}
