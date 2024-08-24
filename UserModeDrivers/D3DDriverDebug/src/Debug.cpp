#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ConPrinter.hpp>

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

static void TestLoadDriver()
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
    DWORD keyNameSize = sizeof(keyNameBuffer);

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
        RegCloseKey(pciKey);
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

    RegCloseKey(pciKey);

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
        RegCloseKey(pciSubKey);
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

    RegCloseKey(pciSubKey);

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

    RegCloseKey(controlClassKey);

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
        RegCloseKey(driverInfoKey);
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

    RegCloseKey(driverInfoKey);

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
    delete[] umdValue;

    ConPrinter::PrintLn(u8"Driver Handle: {}", driverHandle);

    if(!driverHandle)
    {
        LogError(GetLastError());
    }
}

static HMODULE GetDriverHandle()
{
    return GetModuleHandleA("D3DUserModeDriver.dll");
}

extern "C" int EnableD3DDebugLogging(const int enable)
{
    TestLoadDriver();

    HMODULE driver = GetDriverHandle();

    if(!driver)
    {
        return 0;
    }

    using EnableD3DDebugLogging_f = int(*)(int);

    const auto enableD3DDebugLogging = reinterpret_cast<EnableD3DDebugLogging_f>(GetProcAddress(driver, "EnableD3DDebugLogging"));  // NOLINT(clang-diagnostic-cast-function-type-strict)

    return enableD3DDebugLogging(enable);
}

