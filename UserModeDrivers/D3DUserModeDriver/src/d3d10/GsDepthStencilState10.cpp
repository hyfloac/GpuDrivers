#include "d3d10/GsDepthStencilState10.hpp"

GsDepthStencilState10::GsDepthStencilState10(
    const D3D10_DDI_DEPTH_STENCIL_DESC& depthStencilDesc, 
    const D3D10DDI_HRTDEPTHSTENCILSTATE runtimeHandle
) noexcept
    : m_DepthStencilDesc(depthStencilDesc)
    , m_RuntimeHandle(runtimeHandle)
{ }
