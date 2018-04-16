#include <crtdbg.h>
#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE /*hinstance*/, DWORD reason, LPVOID /*reserved*/)
{
	if (DLL_PROCESS_ATTACH == reason)
		_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	return TRUE;
}
