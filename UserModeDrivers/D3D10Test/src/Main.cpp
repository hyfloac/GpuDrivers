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
    ConPrinter::PrintLn(u8"  --d3d10: Run in D3D10 mode.");
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
            else if(strcmp(args[i], "--d3d10") == 0)
            {
                d3dVersion = 10;
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

static LRESULT CALLBACK StaticWindowProc(HWND hWnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) noexcept;

static HWND StartWindow(const int width, const int height) noexcept
{
    WNDCLASSEXW windowClass { };

    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = CS_DBLCLKS;
    windowClass.lpfnWndProc = StaticWindowProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = GetModuleHandleW(nullptr);
    windowClass.hIcon = nullptr;
    windowClass.hCursor = nullptr;
    windowClass.hbrBackground = nullptr;
    windowClass.lpszMenuName = nullptr;
    windowClass.lpszClassName = L"GsWndCls";
    windowClass.hIconSm = nullptr;

    RegisterClassExW(&windowClass);

    HWND hWnd = CreateWindowExW(
        0,
        windowClass.lpszClassName,
        L"D3D10 Test",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        800,
        600,
        nullptr,
        nullptr,
        windowClass.hInstance,
        nullptr
    );

    return hWnd;
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

    constexpr int Width = 800;
    constexpr int Height = 600;

    HWND hWnd = StartWindow(Width, Height);

    EnableD3DDebugLogging(1);

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
        (void) adapter->GetDesc1(&desc);

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

        return 2;
    }

    ConPrinter::PrintLn(u8"Device successfully created.");

    // EnableD3DDebugLogging(1);

    adapter.Release();

    // (void) ShowWindow(hWnd, SW_SHOWNOACTIVATE);

    DXGI_SWAP_CHAIN_DESC swapChainDesc {};
    swapChainDesc.BufferDesc.Width = Width;
    swapChainDesc.BufferDesc.Height = Height;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.Windowed = true;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    CComPtr<IDXGISwapChain> swapChain;
    status = dxgiFactory->CreateSwapChain(device, &swapChainDesc, &swapChain);

    if(!SUCCEEDED(status))
    {
        ConPrinter::PrintLn(u8"Failed to create Swap Chain: 0x{XP0}", status);
        LogError(status);

        return 3;
    }

    CComPtr<ID3D10RenderTargetView> backBufferRtv;

    {
        CComPtr<ID3D10Texture2D> backBuffer;

        status = swapChain->GetBuffer(0, IID_ID3D10Texture2D, reinterpret_cast<void**>(&backBuffer));

        if(!SUCCEEDED(status))
        {
            ConPrinter::PrintLn(u8"Failed to get back buffer: 0x{XP0}", status);
            LogError(status);

            return 4;
        }

        status = device->CreateRenderTargetView(backBuffer, nullptr, &backBufferRtv);

        if(!SUCCEEDED(status))
        {
            ConPrinter::PrintLn(u8"Failed to create render target view for back buffer: 0x{XP0}", status);
            LogError(status);

            return 5;
        }

        device->OMSetRenderTargets(1, &(backBufferRtv.p), nullptr);
    }

    return 0;
}

static LRESULT CALLBACK StaticWindowProc(HWND hWnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) noexcept
{
    if(uMsg == WM_NCCREATE)
    {
        const CREATESTRUCTW* const createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);

        // if(!createStruct->lpCreateParams)
        // {
        //     return FALSE;
        // }

        (void) SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createStruct->lpCreateParams));
    }

    switch(uMsg)
    {
        case WM_CREATE:
        case WM_ACTIVATE:
        case WM_SIZE:
        case WM_MOVE:
            break;
        default: return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
    return 0;

    // Window* const windowPtr = reinterpret_cast<Window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

    // if(!windowPtr)
    // {
    //     return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    // }

    // return windowPtr->WindowProc(hWnd, uMsg, wParam, lParam);
}
