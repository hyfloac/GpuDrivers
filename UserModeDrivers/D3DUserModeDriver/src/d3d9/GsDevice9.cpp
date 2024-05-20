#include "d3d9/GsDevice9.hpp"

GsDevice9::GsDevice9(
    const HANDLE driverHandle, 
    const D3DDDI_DEVICECALLBACKS& deviceCallbacks
) noexcept
    : m_DriverHandle(driverHandle)
    , m_DeviceCallbacks(deviceCallbacks)
{ }
