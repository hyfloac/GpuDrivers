#pragma once

#include "Common.h"

#include "Objects.hpp"

class HyMiniportDevice;

struct GsPresentData final
{
    LIST_ENTRY ListEntry;
    PMDL Mdl;
    ULONG NumMoves;
    ULONG NumDirtyRects;
    D3DKMT_MOVE_RECT* pMoves;
    RECT* pDirtyRect;
    D3DKMDT_VIDPN_PRESENT_PATH_ROTATION Rotation;
    BYTE* Destination;
    BYTE* Source;
    UINT DestPitch;
    UINT PresentPitch;
    UINT DestBPP;
    UINT DestWidth;
    UINT DestHeight;
    D3DDDI_VIDEO_PRESENT_SOURCE_ID SourceId;
public:
    void* operator new(SIZE_T count);
    void operator delete(void* const ptr);
};

class GsPresentManager final
{
    DELETE_CM(GsPresentManager);
public:
    void* operator new(SIZE_T count);
    void operator delete(void* const ptr);
public:
    GsPresentManager(HyMiniportDevice* const deviceContext) noexcept;

    ~GsPresentManager() noexcept
    {
        (void) Close();
    }

    NTSTATUS Init() noexcept;
    NTSTATUS Close() noexcept;
    void FlushPipeline() noexcept;

    void InsertPresent(
        const void* const pPresentDisplayOnlyVoid, 
        PMDL mdl,
        D3DKMDT_VIDPN_PRESENT_PATH_ROTATION Rotation,
        BYTE* Destination,
        BYTE* const Source,
        UINT DestBPP,
        UINT DestPitch,
        UINT DestWidth,
        UINT DestHeight
    ) noexcept;
private:
    [[nodiscard]] GsPresentData* PopPresent(const bool force = false) noexcept;

    void StopThread() noexcept;
    bool IsCloseRequested() noexcept;
    void WakeThread() noexcept;

    void Present(const GsPresentData& data) noexcept;

    void ThreadFunc() noexcept;
private:
    static void StaticThreadFunc(HANDLE Context) noexcept;
private:
    HyMiniportDevice* m_DeviceContext;

    volatile LONG m_PresentThreadShouldStop;
    LIST_ENTRY m_PendingPresentQueue;
    KSPIN_LOCK m_PresentQueueLock;
    KSEMAPHORE m_PresentQueueSemaphore;
    PETHREAD m_PresentThreadObject;
    volatile LONG m_QueuedPresentCount;
    bool m_Started;
};
