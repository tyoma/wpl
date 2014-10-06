#include <windows.h>
#include <commctrl.h>

void init_platform()
{
	INITCOMMONCONTROLSEX icc = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES };

	::InitCommonControlsEx(&icc);
}

void run_message_loop()
{
	MSG msg;

	while (::GetMessage(&msg, NULL, 0, 0))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}

void exit_message_loop()
{
	::PostQuitMessage(0);
}
