#pragma once

#include "Common.hpp"
#include <Objects.hpp>


class GsAdapter9 final
{
    DEFAULT_DESTRUCT(GsAdapter9);
    DELETE_CM(GsAdapter9);
public:
    static GsAdapter9* FromHandle(HANDLE adapter) noexcept
    {
        return static_cast<GsAdapter9*>(adapter);
    }
public:
    GsAdapter9(
        const HANDLE driverHandle,
        const D3DDDI_ADAPTERCALLBACKS& adapterCallbacks
    ) noexcept;

    HRESULT GetCaps(const D3DDDIARG_GETCAPS& getCaps) noexcept;
    HRESULT CreateDevice(D3DDDIARG_CREATEDEVICE& createDevice) noexcept;
private:
    HRESULT GetCapsDDraw(const D3DDDIARG_GETCAPS& getCaps) noexcept;
    HRESULT GetCapsDDrawModeSpecific(const D3DDDIARG_GETCAPS& getCaps) noexcept;
    HRESULT GetCapsD3D9(const D3DDDIARG_GETCAPS& getCaps) noexcept;
private:
    HANDLE m_DriverHandle;
    const D3DDDI_ADAPTERCALLBACKS m_AdapterCallbacks;
};
