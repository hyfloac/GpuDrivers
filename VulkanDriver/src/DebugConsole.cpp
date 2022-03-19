#include <Windows.h>
#include <cstdio>
#include <io.h>

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpReserved)
{
	(void) hInstance;
	(void) fdwReason;
	(void) lpReserved;

#ifdef _DEBUG
	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			// AllocConsole();
			break;
		case DLL_PROCESS_DETACH:
			// FreeConsole();
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		default:
			break;
	}
#endif

	return TRUE;
}
