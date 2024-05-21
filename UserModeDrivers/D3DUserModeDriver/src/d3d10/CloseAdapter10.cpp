// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3d10umddi/nc-d3d10umddi-pfnd3d10ddi_closeadapter
#include "d3d10/CloseAdapter10.hpp"
#include "d3d10/GsAdapter10.hpp"

HRESULT GsCloseAdapterD3D10(D3D10DDI_HADAPTER hAdapter)
{
    if(!hAdapter.pDrvPrivate)
    {
        return E_INVALIDARG;
    }

    delete GsAdapter10::FromHandle(hAdapter);
    return S_OK;
}
