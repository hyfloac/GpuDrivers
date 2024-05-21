#include "d3d9/GsDevice9.hpp"

GsDevice9::GsDevice9(
    const HANDLE driverHandle, 
    const D3DDDI_DEVICECALLBACKS& deviceCallbacks
) noexcept
    : m_DriverHandle(driverHandle)
    , m_DeviceCallbacks(deviceCallbacks)
    , m_RenderState()
{ }

HRESULT GsDevice9::SetRenderState(const D3DDDIARG_RENDERSTATE& renderState) noexcept
{
    switch(renderState.State)
    {
        case D3DDDIRS_ZENABLE: m_RenderState.ZEnable = renderState.Value; break;
        case D3DDDIRS_FILLMODE: m_RenderState.FillMode = renderState.Value; break;
        case D3DDDIRS_SHADEMODE: m_RenderState.ShadeMode = renderState.Value; break;
        // TODO: Fill out the rest of these.
        default: break;
    }

    return S_OK;
}
