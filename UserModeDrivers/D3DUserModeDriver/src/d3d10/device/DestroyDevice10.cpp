// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_destroydevice
#include "d3d10/device/DestroyDevice10.hpp"
#include "d3d10/GsDevice10.hpp"
#include "Logging.hpp"

void APIENTRY GsDestroyDevice10(D3D10DDI_HDEVICE hDevice)
{
#if ENABLE_DEBUG_LOGGING
    if(g_DebugEnable)
    {
        TRACE_ENTRYPOINT_ARG(
            u8"hDevice: 0x{XP0}",
            hDevice.pDrvPrivate
        );
    }
#endif

    if(!hDevice.pDrvPrivate)
    {
        return;
    }

    GsDevice10::FromHandle(hDevice)->~GsDevice10();
}

