#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <ConPrinter.hpp>
#include <Windows.h>
#include <Console.hpp>

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    (void) hinstDLL;
    // Perform actions based on the reason for calling.
    switch(fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // As a DLL create a Console window.
            // Console::Create();
            // Initialize the Console window to the correct settings.
            // Console::Init();
            // Initialize once for each new process.
            // Return FALSE to fail DLL load.
            // ConPrinter::PrintLn(u8"Attached D3DUserModeDriver");
            break;

        case DLL_THREAD_ATTACH:
            // Do thread-specific initialization.
            break;

        case DLL_THREAD_DETACH:
            // Do thread-specific cleanup.
            break;

        case DLL_PROCESS_DETACH:
            // Reset the Console settings back to their original form.
            // Console::Reset();
            // Close the Console window if we created it.
            // Console::Close();

            if(lpvReserved)
            {
                break; // do not do cleanup if process termination scenario
            }

            // Perform any necessary cleanup.
            break;
        default: break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
