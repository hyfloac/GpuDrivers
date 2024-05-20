#include "Common.hpp"
#include <ConPrinter.hpp>

#include "d3d9/CreateDevice.hpp"
#include "d3d9/GetCaps.hpp"
#include "d3d9/GsAdapter9.hpp"


extern "C" HRESULT __declspec(dllexport) OpenAdapter(D3DDDIARG_OPENADAPTER* const pOpenAdapter)
{
    if(!pOpenAdapter)
    {
        return E_INVALIDARG;
    }


    static wchar_t fileName[MAX_PATH + 1];
    (void) GetModuleFileNameW(nullptr, fileName, static_cast<DWORD>(std::size(fileName)));
    
    ConPrinter::PrintLn(u8"OpenAdapter: {}", fileName);

    // D3DDDICB_QUERYADAPTERINFO queryAdapter { };
    // pOpenAdapter->pAdapterCallbacks->pfnQueryAdapterInfoCb(pOpenAdapter->hAdapter, &queryAdapter);

    GsAdapter9* adapter = new GsAdapter9(pOpenAdapter->hAdapter, *pOpenAdapter->pAdapterCallbacks);

    pOpenAdapter->hAdapter = adapter;

    pOpenAdapter->pAdapterFuncs->pfnGetCaps = GsGetCapsD3D9;
    pOpenAdapter->pAdapterFuncs->pfnCreateDevice = GsCreateDeviceD3D9;

    pOpenAdapter->DriverVersion = D3D_UMD_INTERFACE_VERSION;

    return S_OK;
}

extern "C" HRESULT __declspec(dllexport) OpenAdapter10(D3D10DDIARG_OPENADAPTER* const pOpenAdapter)
{
    if(!pOpenAdapter)
    {
        return E_INVALIDARG;
    }

    if((pOpenAdapter->Interface >> 16) < 10)
    {
        return E_INVALIDARG;
    }

    static wchar_t fileName[MAX_PATH + 1];
    (void) GetModuleFileNameW(nullptr, fileName, static_cast<DWORD>(std::size(fileName)));
    
    ConPrinter::PrintLn(u8"OpenAdapter10: {}", fileName);

    pOpenAdapter->pAdapterFuncs->pfnCalcPrivateDeviceSize = nullptr;


    // pOpenAdapter->pAdapterCallbacks->pfnQueryAdapterInfoCb();

    return S_OK;
}
