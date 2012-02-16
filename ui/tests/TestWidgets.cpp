#include "TestWidgets.h"

#include <tchar.h>
#include <windows.h>

using namespace std;
using namespace wpl::ui;

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

	shared_ptr<container::widget_site> TestNativeWidget::set_parent(HWND parent)
	{
		class site : public container::widget_site
		{
			HWND _hwnd;

			virtual void move(int left, int top, int width, int height)
			{	::MoveWindow(_hwnd, left, top, width, height, TRUE);	}

		public:
			site(HWND hwnd)
				: _hwnd(hwnd)
			{	}
		};

		::SetParent(_hwnd, parent);
		::SetWindowLong(_hwnd, GWL_STYLE, (::GetWindowLong(_hwnd, GWL_STYLE) & ~WS_CHILD) | (parent ? WS_CHILD : 0));
		return shared_ptr<container::widget_site>(new site(_hwnd));
	}
}
