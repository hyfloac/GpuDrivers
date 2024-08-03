#include <Console.hpp>
#include <ConPrinter.hpp>
#include <d3d9.h>
#include <d3d10.h>
#include <dxgi.h>
#include <atlbase.h>
#include <new>
#include <D3DDebug.h>

void PrintHelp(int argCount, char* args[]) noexcept
{
    (void) argCount;

    ConPrinter::PrintLn(u8"{} Usage:", args[0]);
    ConPrinter::PrintLn(u8"Performs some tests for the SoftGpu D3D User Mode Driver.");
    ConPrinter::PrintLn();
    ConPrinter::PrintLn(u8"  --help:");
    ConPrinter::PrintLn(u8"  -h: Print this message.");
    ConPrinter::PrintLn(u8"  --debug: Enables debug mode for D3D.");
    ConPrinter::PrintLn(u8"  --d3d9: Run in D3D9 mode.");
}

// From https://learn.microsoft.com/en-us/windows/win32/seccrypto/retrieving-error-messages
static void LogError(const DWORD dwErr) noexcept
{
    WCHAR   wszMsgBuff[512];  // Buffer for text.

    // Try to get the message from the system errors.
    DWORD dwChars = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwErr,
        0,
        wszMsgBuff,
        512,
        NULL);

    if(0 == dwChars)
    {
        // The error code did not exist in the system errors.
        // Try Ntdsbmsg.dll for the error code.

        // Load the library.
        HINSTANCE hInst = LoadLibraryW(L"Ntdsbmsg.dll");
        if(NULL == hInst)
        {
            ConPrinter::PrintLn(u8"Could load Ntdsbmsg.dll");
            return;
        }

        // Try getting message text from ntdsbmsg.
        dwChars = FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            hInst,
            dwErr,
            0,
            wszMsgBuff,
            512,
            NULL);

        // Free the library.
        FreeLibrary(hInst);
    }

    ConPrinter::PrintLn("{}", wszMsgBuff);
}

int RunD3D9(const bool enableDebug) noexcept;
int RunD3D10(const bool enableDebug) noexcept;

int main(int argCount, char* args[])
{
    (void) argCount;
    (void) args;

    bool enableDebug = false;
    int d3dVersion = 10;

    // As a DLL create a Console window.
    Console::Create();
    // Initialize the Console window to the correct settings.
    Console::Init();

    if(argCount > 1)
    {
        for(int i = 1; i < argCount; ++i)
        {
            if(strcmp(args[i], "--debug") == 0)
            {
                enableDebug = true;
            }
            else if(strcmp(args[i], "--help") == 0 || strcmp(args[i], "-h") == 0)
            {
                PrintHelp(argCount, args);
                return 0;
            }
            else if(strcmp(args[i], "--d3d9") == 0)
            {
                d3dVersion = 9;
            }
        }
    }

    // It appears our device is not being presented because it is not active
    // https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-display_devicea
    // This is just a guess at what D3D9 is doing...

    DISPLAY_DEVICEA displayDevice;
    displayDevice.cb = sizeof(DISPLAY_DEVICEA);
    for(UINT i = 0; EnumDisplayDevicesA(nullptr, i, &displayDevice, 0); ++i)
    {
        ConPrinter::PrintLn("Device {} - {}, Flags: {}", i, displayDevice.DeviceString, displayDevice.StateFlags);
    }

    if(d3dVersion == 9)
    {
        return RunD3D9(enableDebug);
    }
    else if(d3dVersion == 10)
    {
        return RunD3D10(enableDebug);
    }

    return 0;
}

int RunD3D9(const bool enableDebug) noexcept
{
    OutputDebugStringA("Starting D3D9 Tester.\n");
    ConPrinter::PrintLn(u8"Starting D3D9 Tester.");


    CComPtr<IDirect3D9> direct3D9 = Direct3DCreate9(D3D_SDK_VERSION);

    if(!direct3D9)
    {
        ConPrinter::PrintLn("Failed to initialize D3D9.");
        return 1;
    }

    EnableD3DDebugLogging(1);

    const UINT adapterCount = direct3D9->GetAdapterCount();

    ConPrinter::PrintLn("Adapter Count: {}", adapterCount);

    D3DADAPTER_IDENTIFIER9 adapter {};
    for(UINT i = 0; i < adapterCount; ++i)
    {
        if(!SUCCEEDED(direct3D9->GetAdapterIdentifier(i, 0, &adapter)))
        {
            ConPrinter::PrintLn("Failed to get adapter {}.", i);
            continue;
        }

        ConPrinter::PrintLn("Adapter: {} - {}, Vendor: 0x{X}, Device 0x{X}", i, adapter.Description, adapter.VendorId, adapter.DeviceId);

        if(adapter.VendorId == 0xFFFD && adapter.DeviceId == 0x0001)
        {
            break;
        }
    }

    return 0;
}

int RunD3D10(const bool enableDebug) noexcept
{
    OutputDebugStringA("Starting D3D10 Tester.\n");
    ConPrinter::PrintLn(u8"Starting D3D10 Tester.");

    CComPtr<IDXGIFactory1> dxgiFactory;

    HRESULT status = CreateDXGIFactory1(IID_IDXGIFactory1, reinterpret_cast<void**>(&dxgiFactory));

    if(!SUCCEEDED(status))
    {
        ConPrinter::PrintLn(u8"Failed to create IDXGIFactory1: 0x{XP0}", status);
        LogError(status);
        return 1;
    }

    CComPtr<IDXGIAdapter1> adapter;
    for(UINT i = 0; dxgiFactory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        ConPrinter::PrintLn(u8"Desc: {}, Vendor: 0x{X}, Device: 0x{X}", desc.Description, desc.VendorId, desc.DeviceId);

        if(desc.VendorId == 0xFFFD)
        {
            break;
        }

        adapter.Release();
    }

    const UINT createDeviceFlags = enableDebug ? D3D10_CREATE_DEVICE_DEBUG : 0;

    CComPtr<ID3D10Device> device;
    status = D3D10CreateDevice(
        adapter,                 // pAdapter
        D3D10_DRIVER_TYPE_HARDWARE, // DriverType
        nullptr,                    // Software
        createDeviceFlags,          // Flags
        D3D10_SDK_VERSION,          // SDKVersion
        &device                     // ppDevice
    );

    if(!SUCCEEDED(status))
    {
        ConPrinter::PrintLn(u8"Failed to create ID3D10Device: 0x{XP0}", status);
        LogError(status);

        // TestLoadDriver();

        Sleep(10000);

        return 2;
    }

    EnableD3DDebugLogging(1);

    adapter.Release();

    ConPrinter::PrintLn(u8"Device successfully created.");

    return 0;
}
