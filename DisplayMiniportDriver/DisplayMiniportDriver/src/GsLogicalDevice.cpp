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

#pragma code_seg()


