#include "d3d10/GsBlendState10.hpp"

GsBlendState10::GsBlendState10(
    const D3D10_DDI_BLEND_DESC& blendDesc,
    const D3D10DDI_HRTBLENDSTATE runtimeHandle
) noexcept
    : m_BlendDesc(blendDesc)
    , m_RuntimeHandle(runtimeHandle)
{ }
