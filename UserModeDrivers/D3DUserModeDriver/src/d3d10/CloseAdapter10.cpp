// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_closeadapter
#include "d3d10/CloseAdapter10.hpp"
#include "d3d10/GsAdapter10.hpp"
#include "Logging.hpp"

HRESULT GsCloseAdapterD3D10(
    const D3D10DDI_HADAPTER hAdapter
)
{
    TRACE_ENTRYPOINT_ARG(
        u8"hAdapter: 0x{XP0}",
        hAdapter.pDrvPrivate
    );

    if(!hAdapter.pDrvPrivate)
    {
        LOG_ERROR(u8"hAdapter was not set.");
        return E_INVALIDARG;
    }

    delete GsAdapter10::FromHandle(hAdapter);
    return S_OK;
}
