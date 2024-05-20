#include <Console.hpp>
#include <ConPrinter.hpp>
#include <d3d10.h>
#include <dxgi.h>
#include <atlbase.h>
#include <new>

void PrintHelp(int argCount, char* args[]) noexcept
{
    (void) argCount;

    ConPrinter::PrintLn(u8"{} Usage:", args[0]);
    ConPrinter::PrintLn(u8"Performs some tests for the SoftGpu D3D User Mode Driver.");
    ConPrinter::PrintLn();
    ConPrinter::PrintLn(u8"  --help:");
    ConPrinter::PrintLn(u8"  -h: Print this message.");
    ConPrinter::PrintLn(u8"  --debug: Enables debug mode for D3D.");
}

// From https://learn.microsoft.com/en-us/windows/win32/seccrypto/retrieving-error-messages
void LogError(const DWORD dwErr) noexcept
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

void TestLoadDriver()
{
    HKEY pciKey;

    LSTATUS status = RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        R"(SYSTEM\ControlSet001\Enum\PCI\VEN_FFFD&DEV_0001&SUBSYS_00000000&REV_01)",
        0,
        KEY_READ,
        &pciKey
    );

    if(status != ERROR_SUCCESS)
    {
        if(status == ERROR_FILE_NOT_FOUND)
        {
            ConPrinter::PrintLn("Failed to find PCI device key.");
        }
        else
        {
            ConPrinter::PrintLn("Failed to load PCI device key.");
        }

        return;
    }

    char keyNameBuffer[256];
    DWORD keyNameSize;

    status = RegEnumKeyExA(
        pciKey,
        0,
        keyNameBuffer,
        &keyNameSize,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    );

    if(status != ERROR_SUCCESS)
    {
        ConPrinter::PrintLn("Failed to enumerate PCI device sub-key.");
        return;
    }

    HKEY pciSubKey;
    status = RegOpenKeyExA(
        pciKey,
        keyNameBuffer,
        0,
        KEY_READ,
        &pciSubKey
    );

    if(status != ERROR_SUCCESS)
    {
        if(status == ERROR_FILE_NOT_FOUND)
        {
            ConPrinter::PrintLn("Failed to find PCI device sub-key.");
        }
        else
        {
            ConPrinter::PrintLn("Failed to load PCI device sub-key.");
        }

        return;
    }

    DWORD driverValueType;
    DWORD driverValueLength;

    status = RegQueryValueExA(
        pciSubKey,
        "Driver",
        nullptr,
        &driverValueType,
        nullptr,
        &driverValueLength
    );

    if(status != ERROR_SUCCESS)
    {
        if(status == ERROR_FILE_NOT_FOUND)
        {
            ConPrinter::PrintLn("Failed to find Driver value.");
        }
        else
        {
            ConPrinter::PrintLn("Failed to get Driver value length.");
        }

        return;
    }

    char* driverValue = new(::std::nothrow) char[driverValueLength];

    if(!driverValue)
    {
        ConPrinter::PrintLn("Failed to allocate buffer for Driver value.");
        return;
    }

    status = RegQueryValueExA(
        pciSubKey,
        "Driver",
        nullptr,
        &driverValueType,
        reinterpret_cast<LPBYTE>(driverValue),
        &driverValueLength
    );

    if(status != ERROR_SUCCESS)
    {
        delete[] driverValue;

        if(status == ERROR_FILE_NOT_FOUND)
        {
            ConPrinter::PrintLn("Failed to find Driver value.");
        }
        else
        {
            ConPrinter::PrintLn("Failed to get Driver value.");
        }

        return;
    }

    HKEY controlClassKey;

    status = RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        R"(SYSTEM\ControlSet001\Control\Class)",
        0,
        KEY_READ,
        &controlClassKey
    );

    if(status != ERROR_SUCCESS)
    {
        delete[] driverValue;

        if(status == ERROR_FILE_NOT_FOUND)
        {
            ConPrinter::PrintLn("Failed to find Control\\Class key.");
        }
        else
        {
            ConPrinter::PrintLn("Failed to load Control\\Class key.");
        }

        return;
    }

    HKEY driverInfoKey;

    status = RegOpenKeyExA(
        controlClassKey,
        driverValue,
        0,
        KEY_READ,
        &driverInfoKey
    );

    delete[] driverValue;
    driverValue = nullptr;

    if(status != ERROR_SUCCESS)
    {
        if(status == ERROR_FILE_NOT_FOUND)
        {
            ConPrinter::PrintLn("Failed to find Driver Info key.");
        }
        else
        {
            ConPrinter::PrintLn("Failed to load Driver Info key.");
        }

        return;
    }

    DWORD umdValueType;
    DWORD umdValueLength;

    status = RegQueryValueExA(
        driverInfoKey,
        "UserModeDriverName",
        nullptr,
        &umdValueType,
        nullptr,
        &umdValueLength
    );

    if(status != ERROR_SUCCESS)
    {
        if(status == ERROR_FILE_NOT_FOUND)
        {
            ConPrinter::PrintLn("Failed to find UserModeDriverName value.");
        }
        else
        {
            ConPrinter::PrintLn("Failed to get UserModeDriverName value length.");
        }

        return;
    }

    char* umdValue = new(::std::nothrow) char[umdValueLength];

    if(!umdValue)
    {
        ConPrinter::PrintLn("Failed to allocate buffer for UserModeDriverName value.");
        return;
    }

    status = RegQueryValueExA(
        driverInfoKey,
        "UserModeDriverName",
        nullptr,
        &umdValueType,
        reinterpret_cast<LPBYTE>(umdValue),
        &umdValueLength
    );

    if(status != ERROR_SUCCESS)
    {
        delete[] umdValue;

        if(status == ERROR_FILE_NOT_FOUND)
        {
            ConPrinter::PrintLn("Failed to find UserModeDriverName value.");
        }
        else
        {
            ConPrinter::PrintLn("Failed to get UserModeDriverName value.");
        }

        return;
    }

    ConPrinter::PrintLn(u8"Driver Path: {}", umdValue);

    HANDLE driverHandle = LoadLibraryA(umdValue);

    ConPrinter::PrintLn(u8"Driver Handle: {}", driverHandle);

    LogError(GetLastError());
}

int main(int argCount, char* args[])
{
    (void) argCount;
    (void) args;

    bool enableDebug = false;

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
        }
    }

    OutputDebugStringA("Starting D3D10 Tester.\n");

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

    adapter.Release();


    ConPrinter::PrintLn(u8"Device successfully created.");

    Sleep(5000);

    return 0;
}
