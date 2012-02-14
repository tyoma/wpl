#include "TestWidgets.h"

#include <tchar.h>
#include <windows.h>

namespace ut
{
	TestNativeWidget::TestNativeWidget()
		: _hwnd(::CreateWindow(_T("static"), NULL, NULL, 0, 0, 50, 50, NULL, NULL, NULL, NULL))
	{
	}

	TestNativeWidget::~TestNativeWidget()
	{
		::DestroyWindow(_hwnd);
	}

	void TestNativeWidget::set_parent(HWND parent)
	{
		::SetParent(_hwnd, parent);
		::SetWindowLong(_hwnd, GWL_STYLE, (::GetWindowLong(_hwnd, GWL_STYLE) & ~WS_CHILD) | (parent ? WS_CHILD : 0));
	}
}
