// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_createdevice
#include "d3d10/CreateDevice10.hpp"
#include "d3d10/GsAdapter10.hpp"
#include "Logging.hpp"

HRESULT GsCreateDeviceD3D10(D3D10DDI_HADAPTER hAdapter, D3D10DDIARG_CREATEDEVICE* const pCreateDevice)
{
#if ENABLE_DEBUG_LOGGING
    if(g_DebugEnable)
    {
        TRACE_ENTRYPOINT_ARG(
            u8"hRTDevice: 0x{XP0}, Interface: 0x{XP}, Version 0x{X}, hDrvDevice: 0x{XP0}, hRTCoreLayer: 0x{XP0}, Flags: 0x{XP0}",
            pCreateDevice->hRTDevice.handle,
            pCreateDevice->Interface,
            pCreateDevice->Version,
            pCreateDevice->hDrvDevice.pDrvPrivate,
            pCreateDevice->hRTCoreLayer.handle,
            pCreateDevice->Flags
        );
    }
#endif

    if(!hAdapter.pDrvPrivate)
    {
        return E_INVALIDARG;
    }

    if(!pCreateDevice)
    {
        return E_INVALIDARG;
    }

    return GsAdapter10::FromHandle(hAdapter)->CreateDevice(*pCreateDevice);
}
