#include "d3d10/GsDevice10.hpp"

GsDevice10::GsDevice10(
    const D3D10DDI_HRTDEVICE driverHandle, 
    const D3DDDI_DEVICECALLBACKS& deviceCallbacks,
    const D3D10DDI_CORELAYER_DEVICECALLBACKS& umCallbacks
) noexcept
    : m_DriverHandle(driverHandle)
    , m_DeviceCallbacks(deviceCallbacks)
    , m_UmCallbacks(umCallbacks)
{ }
