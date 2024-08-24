// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_createblendstate
#include "d3d10/device/CreateBlendState10.hpp"
#include "d3d10/GsDevice10.hpp"
#include "Logging.hpp"

void APIENTRY GsCreateBlendState10(
    D3D10DDI_HDEVICE hDevice,
    const D3D10_DDI_BLEND_DESC* pBlendDesc,
    D3D10DDI_HBLENDSTATE hBlendState,
    D3D10DDI_HRTBLENDSTATE hRtBlendState
)
{
#if ENABLE_DEBUG_LOGGING
    if(g_DebugEnable)
    {
        TRACE_ENTRYPOINT_ARG(
            u8"hDevice: 0x{XP0}, pBlendDesc: 0x{XP0}, hBlendState: 0x{XP0}, hRtBlendState: 0x{XP0}",
            hDevice.pDrvPrivate,
            pBlendDesc,
            hBlendState.pDrvPrivate,
            hRtBlendState.handle
        );
    }
#endif

    if(!hDevice.pDrvPrivate)
    {
        return;
    }

    GsDevice10::FromHandle(hDevice)->CreateBlendState(pBlendDesc, hBlendState, hRtBlendState);
}

