#include "d3d10/GsAdapter10.hpp"
#include "d3d10/GsDevice10.hpp"

GsAdapter10::GsAdapter10(
    const D3D10DDI_HRTADAPTER driverHandle, 
    const D3DDDI_ADAPTERCALLBACKS& adapterCallbacks
) noexcept
    : m_DriverHandle(driverHandle)
    , m_AdapterCallbacks(adapterCallbacks)
{ }

HRESULT GsAdapter10::CreateDevice(D3D10DDIARG_CREATEDEVICE& createDevice) noexcept
{
    GsDevice10* const device = new GsDevice10(
        createDevice.hRTDevice,     // driverHandle
        *createDevice.pKTCallbacks, // deviceCallbacks
        *createDevice.pUMCallbacks  // umCallbacks
    );

    createDevice.hDrvDevice.pDrvPrivate = device;

    return S_OK;
}
