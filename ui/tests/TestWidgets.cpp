#include "TestWidgets.h"

#include <wpl/ui/win32/native_view.h>

#include <tchar.h>
#include <windows.h>

using namespace std;

namespace ut
{
	TestNativeWidget::TestNativeWidget()
		: _hwnd(::CreateWindow(_T("static"), NULL, NULL, 0, 0, 50, 50, NULL, NULL, NULL, NULL))
	{	}

	TestNativeWidget::~TestNativeWidget()
	{	::DestroyWindow(_hwnd);	}

	shared_ptr<wpl::ui::view> TestNativeWidget::create_custom_view()
	{	return shared_ptr<wpl::ui::view>(new wpl::ui::native_view(shared_from_this(), _hwnd));	}
}
