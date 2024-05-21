#pragma once

#include "Common.hpp"
#include <Objects.hpp>

class GsDevice10 final
{
    DEFAULT_DESTRUCT(GsDevice10);
    DELETE_CM(GsDevice10);
public:
    static GsDevice10* FromHandle(D3D10DDI_HADAPTER adapter) noexcept
    {
        return static_cast<GsDevice10*>(adapter.pDrvPrivate);
    }
public:
    GsDevice10(
        const D3D10DDI_HRTDEVICE driverHandle,
        const D3DDDI_DEVICECALLBACKS& deviceCallbacks,
        const D3D10DDI_CORELAYER_DEVICECALLBACKS& umCallbacks
    ) noexcept;
private:
private:
    D3D10DDI_HRTDEVICE m_DriverHandle;
    const D3DDDI_DEVICECALLBACKS m_DeviceCallbacks;
    const D3D10DDI_CORELAYER_DEVICECALLBACKS m_UmCallbacks;
};
