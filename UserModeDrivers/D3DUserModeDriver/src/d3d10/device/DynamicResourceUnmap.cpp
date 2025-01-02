// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_resourceunmap
#include "d3d10/device/DynamicResourceUnmap.hpp"
#include "d3d10/GsDevice10.hpp"
#include "Logging.hpp"

void APIENTRY GsDynamicResourceUnmap(
    const D3D10DDI_HDEVICE hDevice,
    const D3D10DDI_HRESOURCE hResource,
    const UINT Subresource
)
{
    TRACE_ENTRYPOINT_ARG(
        u8"hDevice: 0x{XP0}, hResource: 0x{XP0}, Subresource: {}",
        hDevice.pDrvPrivate,
        hResource.pDrvPrivate,
        Subresource
    );

    if(!hDevice.pDrvPrivate)
    {
        LOG_ERROR(u8"hDevice was not set.");
        return;
    }

    if(!hResource.pDrvPrivate)
    {
        LOG_WARN(u8"hResource was not set.");
        return;
    }

    GsDevice10::FromHandle(hDevice)->DynamicResourceUnmap(
        hResource,
        Subresource
    );
}
