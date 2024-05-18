#include "GsGraphicsContext.hpp"
#include "HyDevice.hpp"
#include "Logging.h"
#include "MemoryAllocator.h"

#pragma code_seg("PAGE")

void* GsGraphicsContext::operator new(SIZE_T count)
{
    return HyAllocate(PagedPool, count, POOL_TAG_GRAPHICS_CONTEXT);
}

void GsGraphicsContext::operator delete(void* ptr)
{
    HyDeallocate(ptr, POOL_TAG_GRAPHICS_CONTEXT);
}

GsGraphicsContext::GsGraphicsContext(
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

#pragma code_seg()


