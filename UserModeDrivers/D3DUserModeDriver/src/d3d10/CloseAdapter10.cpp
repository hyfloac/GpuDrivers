// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_closeadapter
#include "d3d10/CloseAdapter10.hpp"
#include "d3d10/GsAdapter10.hpp"
#include "Logging.hpp"

HRESULT GsCloseAdapterD3D10(D3D10DDI_HADAPTER hAdapter)
{
#if ENABLE_DEBUG_LOGGING
    if(g_DebugEnable)
    {
        TRACE_ENTRYPOINT_ARG(
            u8"hAdapter: 0x{XP0}",
            hAdapter.pDrvPrivate
        );
    }
#endif

    if(!hAdapter.pDrvPrivate)
    {
        return E_INVALIDARG;
    }

    delete GsAdapter10::FromHandle(hAdapter);
    return S_OK;
}
