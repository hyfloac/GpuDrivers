// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_destroyblendstate
#include "d3d10/device/DestroyBlendState10.hpp"
#include "d3d10/GsDevice10.hpp"
#include "Logging.hpp"

void APIENTRY GsDestroyBlendState10(
    const D3D10DDI_HDEVICE hDevice,
    const D3D10DDI_HBLENDSTATE hBlendState
)
{
    TRACE_ENTRYPOINT_ARG(
        u8"hDevice: 0x{XP0}, hBlendState: 0x{XP0}",
        hDevice.pDrvPrivate,
        hBlendState.pDrvPrivate
    );

    if(!hDevice.pDrvPrivate)
    {
        LOG_ERROR(u8"hDevice was not set.");
        return;
    }

    GsDevice10::FromHandle(hDevice)->DestroyBlendState(
        hBlendState
    );
}
