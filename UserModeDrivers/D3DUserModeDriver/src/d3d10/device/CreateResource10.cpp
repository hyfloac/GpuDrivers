// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_createresource
#include "d3d10/device/CreateResource10.hpp"
#include "d3d10/GsDevice10.hpp"
#include "Logging.hpp"

void APIENTRY GsCreateResource10(
    const D3D10DDI_HDEVICE hDevice,
    const D3D10DDIARG_CREATERESOURCE* const pCreateResource,
    const D3D10DDI_HRESOURCE hResource,
    const D3D10DDI_HRTRESOURCE hRtResource
)
{
    TRACE_ENTRYPOINT_ARG(
        u8"hDevice: 0x{XP0}, pCreateResource: 0x{XP0}, hResource: 0x{XP0}, hRtResource: 0x{XP0}",
        hDevice.pDrvPrivate,
        pCreateResource,
        hResource.pDrvPrivate,
        hRtResource.handle
    );

    if(!hDevice.pDrvPrivate)
    {
        LOG_ERROR(u8"hDevice was not set.");
        return;
    }

    if(!pCreateResource)
    {
        LOG_WARN(u8"pCreateResource was not set.");
        // return 0;
    }

    GsDevice10::FromHandle(hDevice)->CreateResource(
        pCreateResource,
        hResource,
        hRtResource
    );
}
