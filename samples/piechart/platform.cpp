#include <windows.h>

#include <tchar.h>
#include <wpl/ui/win32/controls.h>
#include <CommCtrl.h>

using namespace std;
using namespace wpl::ui;

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

void dummy(shared_ptr<void>, shared_ptr<listview>)
{	}

shared_ptr<listview> create_listview()
{
	shared_ptr<void> hlv = shared_ptr<void>(::CreateWindow(_T("SysListView32"), NULL,
		WS_POPUP | WS_VISIBLE | LVS_REPORT | LVS_OWNERDATA, 0, 0, 0, 0, NULL, NULL, NULL, 0), &::DestroyWindow);
	shared_ptr<listview> lv(wrap_listview(static_cast<HWND>(hlv.get())));

	return shared_ptr<listview>(lv.get(), bind(&dummy, hlv, lv));
}
