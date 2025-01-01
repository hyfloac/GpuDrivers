#include "GsLogicalDevice.hpp"
#include "GsGraphicsContext.hpp"
#include "Logging.h"
#include "MemoryAllocator.h"

#pragma code_seg("PAGE")

void* GsLogicalDevice::operator new(SIZE_T count)
{
    return HyAllocate(PagedPool, count, POOL_TAG_GRAPHICS_CONTEXT);
}

void GsLogicalDevice::operator delete(void* ptr)
{
    HyDeallocate(ptr, POOL_TAG_GRAPHICS_CONTEXT);
}

GsLogicalDevice::GsLogicalDevice(
    HANDLE dxgkHandle,
    const bool isSystemDevice,
    const bool isGdiDevice
) noexcept
    : m_DxgkHandle(dxgkHandle)
    , m_IsSystemDevice(isSystemDevice)
    , m_IsGdiDevice(isGdiDevice)
    , m_Pad0(0)
    , m_DeviceInfo()
{
    m_DeviceInfo.DmaBufferSize = 4096;
    // Not sure what to set here
    m_DeviceInfo.DmaBufferSegmentSet = 1;
    m_DeviceInfo.DmaBufferPrivateDataSize = 0;
    m_DeviceInfo.AllocationListSize = 1;
    m_DeviceInfo.PatchLocationListSize = 1;
    m_DeviceInfo.Flags.GuaranteedDmaBufferContract = true;
}

NTSTATUS GsLogicalDevice::CreateContext(INOUT_PDXGKARG_CREATECONTEXT pCreateContext) noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);
    LOG_DEBUG(
        "Node Ordinal: %u, Engine Affinity: %u, System Device: %d, GDI Device: %d\n",
        pCreateContext->NodeOrdinal,
        pCreateContext->EngineAffinity,
        pCreateContext->Flags.SystemContext,
        pCreateContext->Flags.GdiContext
    );

    GsGraphicsContext* graphicsContext = new GsGraphicsContext(
        pCreateContext->NodeOrdinal,         // nodeOrdinal
        pCreateContext->EngineAffinity,      // engineAffinity
        pCreateContext->Flags.SystemContext, // isSystemContext
        pCreateContext->Flags.GdiContext,    // isGdiContext
        false,
        false,
        false,
        false
    );

    if(!graphicsContext)
    {
        LOG_WARN("Failed to allocate Graphics Context.\n");
        return STATUS_NO_MEMORY;
    }

    pCreateContext->hContext = graphicsContext;
    pCreateContext->ContextInfo = graphicsContext->ContextInfo();

    return STATUS_SUCCESS;
}

NTSTATUS GsLogicalDevice::OpenAllocation(IN_CONST_PDXGKARG_OPENALLOCATION pOpenAllocation) noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    if(!pOpenAllocation->Flags.Create)
    {
        return STATUS_SUCCESS;
    }

    if(pOpenAllocation->Flags.ReadOnly)
    {
        return STATUS_SUCCESS;
    }

    // Set the hDeviceSpecificAllocation member of each pOpenAllocation to some metadata structure, or just some dynamically allocated integer for now.

    for(UINT i = 0; i < pOpenAllocation->NumAllocations; ++i)
    {
        DXGK_OPENALLOCATIONINFO& allocation = pOpenAllocation->pOpenAllocation[i];
        {
            UINT* const x = HY_ALLOC(UINT, PagedPool, POOL_TAG_RESOURCE);
            *x = POOL_TAG_RESOURCE + 2;
            allocation.hDeviceSpecificAllocation = x;
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS GsLogicalDevice::CloseAllocation(IN_CONST_PDXGKARG_CLOSEALLOCATION pCloseAllocation) noexcept
{
    CHECK_IRQL(PASSIVE_LEVEL);

    for(UINT i = 0; i < pCloseAllocation->NumAllocations; ++i)
    {
        HY_FREE(pCloseAllocation->pOpenHandleList[i], POOL_TAG_RESOURCE);
    }

    return STATUS_SUCCESS;
}

#pragma code_seg()


