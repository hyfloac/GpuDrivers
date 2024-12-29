// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_calcprivateresourcesize
#include "d3d10/device/CalcPrivateResourceSize10.hpp"
#include "d3d10/GsDevice10.hpp"
#include "Logging.hpp"

SIZE_T APIENTRY GsCalcPrivateResourceSize10(
    const D3D10DDI_HDEVICE hDevice,
    const D3D10DDIARG_CREATERESOURCE* const pCreateResource
)
{
    TRACE_ENTRYPOINT_ARG(
        u8"hDevice: 0x{XP0}, pCreateResource: 0x{XP0}",
        hDevice.pDrvPrivate,
        pCreateResource
    );

    if(!hDevice.pDrvPrivate)
    {
        LOG_ERROR(u8"hDevice was not set.");
        return 0;
    }

    if(!pCreateResource)
    {
        LOG_WARN(u8"pCreateResource was not set.");
        // return 0;
    }

    return GsDevice10::FromHandle(hDevice)->CalcPrivateResourceSize(
        pCreateResource
    );
}
