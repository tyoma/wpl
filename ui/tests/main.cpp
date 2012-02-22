#include <crtdbg.h>
#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE /*hinstance*/, DWORD reason, LPVOID /*reserved*/)
{
	if (DLL_PROCESS_DETACH == reason)
		_CrtDumpMemoryLeaks();
	return TRUE;
}
