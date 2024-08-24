// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_setblendstate
// https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-omsetblendstate
#include "d3d10/device/SetBlendState10.hpp"
#include "d3d10/GsDevice10.hpp"
#include "Logging.hpp"

void APIENTRY GsSetBlendState10(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HBLENDSTATE hBlendState,
    const FLOAT BlendFactor[4],
    UINT SampleMask
)
{
#if ENABLE_DEBUG_LOGGING
    if(g_DebugEnable)
    {
        TRACE_ENTRYPOINT_ARG(
            u8"hDevice: 0x{XP0}, hBlendState: 0x{XP0}, BlendFactor: [{}, {}, {}, {}], SampleMask: 0x{XP0}",
            hDevice.pDrvPrivate,
            hBlendState.pDrvPrivate,
            BlendFactor[0],
            BlendFactor[1],
            BlendFactor[2],
            BlendFactor[3],
            SampleMask
        );
    }
#endif

    if(!hDevice.pDrvPrivate)
    {
        return;
    }

    GsDevice10::FromHandle(hDevice)->SetBlendState(hBlendState, BlendFactor, SampleMask);
}
