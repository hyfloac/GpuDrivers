#pragma once

#include "Common.hpp"
#include <Objects.hpp>

struct GsRenderState9 final
{
public:
    UINT ZEnable;
    UINT FillMode;
    UINT ShadeMode;
};

class GsDevice9 final
{
    DEFAULT_DESTRUCT(GsDevice9);
    DELETE_CM(GsDevice9);
public:
    static GsDevice9* FromHandle(HANDLE adapter) noexcept
    {
        return static_cast<GsDevice9*>(adapter);
    }
public:
    GsDevice9(
        const HANDLE driverHandle,
        const D3DDDI_DEVICECALLBACKS& deviceCallbacks
    ) noexcept;

    HRESULT SetRenderState(const D3DDDIARG_RENDERSTATE& renderState) noexcept;
private:
private:
    HANDLE m_DriverHandle;
    const D3DDDI_DEVICECALLBACKS m_DeviceCallbacks;
    GsRenderState9 m_RenderState;
};
