#include "d3d10/GsResource10.hpp"

GsResource10::GsResource10(
    const D3D10DDI_HRTRESOURCE runtimeHandle,
    const D3DKMT_HANDLE allocationHandle,
    const GsResourceDesc10& desc
) noexcept
    : m_RuntimeHandle(runtimeHandle)
    , m_AllocationHandle(allocationHandle)
    , m_Desc(desc)
    , m_Allocation(nullptr)
{ }
