#pragma once

#include "Common.hpp"
#include <Objects.hpp>

class GsBlendState10 final
{
    DEFAULT_DESTRUCT(GsBlendState10);
    DELETE_CM(GsBlendState10);
public:
    static GsBlendState10* FromHandle(const D3D10DDI_HBLENDSTATE blendState) noexcept
    {
        return static_cast<GsBlendState10*>(blendState.pDrvPrivate);
    }
public:
    GsBlendState10(
        const D3D10_DDI_BLEND_DESC& blendDesc,
        const D3D10DDI_HRTBLENDSTATE runtimeHandle
    ) noexcept;

    [[nodiscard]] const D3D10_DDI_BLEND_DESC& BlendDesc() const noexcept { return m_BlendDesc; }
    [[nodiscard]] D3D10DDI_HRTBLENDSTATE RuntimeHandle() const noexcept { return m_RuntimeHandle; }
private:
    D3D10_DDI_BLEND_DESC m_BlendDesc;
    D3D10DDI_HRTBLENDSTATE m_RuntimeHandle;
};
