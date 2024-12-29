#include "Logging.hpp"
#include <Windows.h>

// From https://learn.microsoft.com/en-us/windows/win32/seccrypto/retrieving-error-messages
void LogWindowsError(const DWORD dwErr) noexcept
{
#if GS_ENABLE_DEBUG_LOGGING
    WCHAR wszMsgBuff[512];  // Buffer for text.

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

    LOG_ERROR("{}", wszMsgBuff);
#endif
}

void LogWindowsHResultAndError(const HRESULT hResult) noexcept
{
    LogWindowsError(hResult);
    LogWindowsError(GetLastError());
}
