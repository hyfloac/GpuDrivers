// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_resourcemap
#include "d3d10/device/DynamicResourceMapDiscard10.hpp"
#include "d3d10/GsDevice10.hpp"
#include "Logging.hpp"

void APIENTRY GsDynamicResourceMapDiscard10(
    const D3D10DDI_HDEVICE hDevice,
    const D3D10DDI_HRESOURCE hResource,
    const UINT Subresource,
    const D3D10_DDI_MAP MapType,
    const UINT MapFlags,
    D3D10DDI_MAPPED_SUBRESOURCE* const pMappedSubresource
)
{
    TRACE_ENTRYPOINT_ARG(
        u8"hDevice: 0x{XP0}, hResource: 0x{XP0}, Subresource: {}, MapType: {}, MapFlags: 0x{XP0}, pMappedSubresource: 0x{XP0}",
        hDevice.pDrvPrivate,
        hResource.pDrvPrivate,
        Subresource,
        MapType,
        MapFlags,
        pMappedSubresource
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

    GsDevice10::FromHandle(hDevice)->DynamicResourceMapDiscard(
        hResource,
        Subresource,
        MapType,
        MapFlags,
        pMappedSubresource
    );
}
