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

GsGraphicsContext::GsGraphicsContext(const bool isSoftwareDevice, const bool isGdiDevice) noexcept
    : m_IsSoftwareDevice(isSoftwareDevice)
    , m_IsGdiDevice(isGdiDevice)
    , m_Pad0(0)
{ }

#pragma code_seg()


