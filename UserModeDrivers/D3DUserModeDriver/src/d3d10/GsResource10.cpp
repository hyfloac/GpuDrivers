#include "d3d10/GsResource10.hpp"

GsResource10::GsResource10(
    const D3D10DDI_HRTRESOURCE runtimeHandle
) noexcept
    : m_RuntimeHandle(runtimeHandle)
{ }
