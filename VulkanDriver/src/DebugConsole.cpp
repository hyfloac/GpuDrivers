#include <Windows.h>
#include <Console.hpp>

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpReserved)
{
	(void) hInstance;
	(void) fdwReason;
	(void) lpReserved;

#ifdef _DEBUG
	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			// As a DLL create a Console window.
			Console::Create();
			// Initialize the Console window to the correct settings.
			Console::Init();
			break;
		case DLL_PROCESS_DETACH:
			// Reset the Console settings back to their original form.
			Console::Reset();
			// Close the Console window if we created it.
			Console::Close();
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		default:
			break;
	}
#endif

	return TRUE;
}
