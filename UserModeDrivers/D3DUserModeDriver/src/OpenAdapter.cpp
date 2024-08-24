#include "Common.hpp"
#include <ConPrinter.hpp>

#include "d3d10/CalcPrivateDeviceSize10.hpp"
#include "d3d10/CloseAdapter10.hpp"
#include "d3d10/CreateDevice10.hpp"
#include "d3d10/GsAdapter10.hpp"
#include "d3d9/CloseAdapter9.hpp"
#include "d3d9/CreateDevice9.hpp"
#include "d3d9/GetCaps9.hpp"
#include "d3d9/GsAdapter9.hpp"

bool g_DebugEnable = false;

extern "C" int __declspec(dllexport) EnableD3DDebugLogging(const int enable)
{
    static bool EnabledOnce = false;

    g_DebugEnable = enable ? true : false;

    if(!EnabledOnce && enable)
    {
        EnabledOnce = true;

        // As a DLL create a Console window.
        Console::Create();
        // Initialize the Console window to the correct settings.
        Console::Init();
    }

    return 0;
}

extern "C" HRESULT __declspec(dllexport) OpenAdapter(D3DDDIARG_OPENADAPTER* const pOpenAdapter)
{
    if(!pOpenAdapter)
    {
        return E_INVALIDARG;
    }

    if(g_DebugEnable)
    {
        static wchar_t fileName[MAX_PATH + 1];
        (void) GetModuleFileNameW(nullptr, fileName, static_cast<DWORD>(std::size(fileName)));

        ConPrinter::PrintLn(u8"OpenAdapter: {}", fileName);
    }

    // D3DDDICB_QUERYADAPTERINFO queryAdapter { };
    // pOpenAdapter->pAdapterCallbacks->pfnQueryAdapterInfoCb(pOpenAdapter->hAdapter, &queryAdapter);

    GsAdapter9* adapter = new GsAdapter9(pOpenAdapter->hAdapter, *pOpenAdapter->pAdapterCallbacks);

    pOpenAdapter->hAdapter = adapter;

    pOpenAdapter->pAdapterFuncs->pfnGetCaps = GsGetCapsD3D9;
    pOpenAdapter->pAdapterFuncs->pfnCreateDevice = GsCreateDeviceD3D9;
    pOpenAdapter->pAdapterFuncs->pfnCloseAdapter = GsCloseAdapterD3D9;

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

    if(g_DebugEnable)
    {
        static wchar_t fileName[MAX_PATH + 1];
        (void) GetModuleFileNameW(nullptr, fileName, static_cast<DWORD>(std::size(fileName)));

        ConPrinter::PrintLn(u8"OpenAdapter10: {}", fileName);
    }

    pOpenAdapter->pAdapterFuncs->pfnCalcPrivateDeviceSize = nullptr;

    GsAdapter10* adapter = new GsAdapter10(pOpenAdapter->hRTAdapter, *pOpenAdapter->pAdapterCallbacks);

    pOpenAdapter->hAdapter.pDrvPrivate = adapter;

    pOpenAdapter->pAdapterFuncs->pfnCalcPrivateDeviceSize = GsCalcPrivateDeviceSizeD3D10;
    pOpenAdapter->pAdapterFuncs->pfnCreateDevice = GsCreateDeviceD3D10;
    pOpenAdapter->pAdapterFuncs->pfnCloseAdapter = GsCloseAdapterD3D10;

    // pOpenAdapter->pAdapterCallbacks->pfnQueryAdapterInfoCb();

    return S_OK;
}
