#include "GsGraphicsContext.hpp"
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
    const UINT nodeOrdinal,
    const UINT engineAffinity,
    const bool isSystemContext,
    const bool isGdiContext,
    const bool isVirtualAddressing,
    const bool isSystemProtectedContext,
    const bool isHwQueueSupported,
    const bool isTestContext
) noexcept
    : m_NodeOrdinal(nodeOrdinal)
    , m_EngineAffinity(engineAffinity)
    , m_IsSystemContext(isSystemContext)
    , m_IsGdiContext(isGdiContext)
    , m_IsVirtualAddressing(isVirtualAddressing)
    , m_IsSystemProtectedContext(isSystemProtectedContext)
    , m_IsHwQueueSupported(isHwQueueSupported)
    , m_IsTestContext(isTestContext)
    , m_Pad0(0)
    , m_ContextInfo()
{
    m_ContextInfo.DmaBufferSize = 4096;
    // Not sure what to set here
    m_ContextInfo.DmaBufferSegmentSet = 1;
    m_ContextInfo.DmaBufferPrivateDataSize = 0;
    m_ContextInfo.AllocationListSize = 1;
    m_ContextInfo.PatchLocationListSize = 1;
}

#pragma code_seg()
