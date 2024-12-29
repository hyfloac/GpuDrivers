#pragma once

#include "Common.hpp"
#include <Objects.hpp>

class GsResource10 final
{
    DEFAULT_DESTRUCT(GsResource10);
    DELETE_CM(GsResource10);
public:
    static GsResource10* FromHandle(const D3D10DDI_HRESOURCE handle) noexcept
    {
        return static_cast<GsResource10*>(handle.pDrvPrivate);
    }
public:
    GsResource10(
        const D3D10DDI_HRTRESOURCE runtimeHandle
    ) noexcept;

    [[nodiscard]] D3D10DDI_HRTRESOURCE RuntimeHandle() const noexcept { return m_RuntimeHandle; }
private:
    D3D10DDI_HRTRESOURCE m_RuntimeHandle;
};
