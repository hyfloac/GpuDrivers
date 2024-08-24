#pragma once

#include "Common.hpp"
#include <Objects.hpp>

class GsDepthStencilState10 final
{
    DEFAULT_DESTRUCT(GsDepthStencilState10);
    DELETE_CM(GsDepthStencilState10);
public:
    static GsDepthStencilState10* FromHandle(const D3D10DDI_HDEPTHSTENCILSTATE depthStencilState) noexcept
    {
        return static_cast<GsDepthStencilState10*>(depthStencilState.pDrvPrivate);
    }
public:
    GsDepthStencilState10(
        const D3D10_DDI_DEPTH_STENCIL_DESC& depthStencilDesc,
        const D3D10DDI_HRTDEPTHSTENCILSTATE runtimeHandle
    ) noexcept;

    [[nodiscard]] const D3D10_DDI_DEPTH_STENCIL_DESC& DepthStencilDesc() const noexcept { return m_DepthStencilDesc; }
    [[nodiscard]] D3D10DDI_HRTDEPTHSTENCILSTATE RuntimeHandle() const noexcept { return m_RuntimeHandle; }
private:
    D3D10_DDI_DEPTH_STENCIL_DESC m_DepthStencilDesc;
    D3D10DDI_HRTDEPTHSTENCILSTATE m_RuntimeHandle;
};
