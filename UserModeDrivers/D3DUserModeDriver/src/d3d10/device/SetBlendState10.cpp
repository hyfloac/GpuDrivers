// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_setblendstate
// See https://learn.microsoft.com/en-us/windows/win32/api/d3d10/nf-d3d10-id3d10device-omsetblendstate
#include "d3d10/device/SetBlendState10.hpp"
#include "d3d10/GsDevice10.hpp"
#include "Logging.hpp"

void APIENTRY GsSetBlendState10(
    const D3D10DDI_HDEVICE hDevice,
    const D3D10DDI_HBLENDSTATE hBlendState,
    const FLOAT BlendFactor[4],
    const UINT SampleMask
)
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

    if(!hDevice.pDrvPrivate)
    {
        LOG_ERROR(u8"hDevice was not set.");
        return;
    }

    GsDevice10::FromHandle(hDevice)->SetBlendState(
        hBlendState, 
        BlendFactor, 
        SampleMask
    );
}
