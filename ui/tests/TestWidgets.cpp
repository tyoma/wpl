#include "TestWidgets.h"

#include <wpl/ui/geometry.h>
#include <wpl/ui/win32/native_view.h>

#include <tchar.h>
#include <windows.h>

using namespace std;
using namespace wpl;
using namespace wpl::ui;

namespace ut
{
	namespace
	{
		class TestView : public native_view, noncopyable
		{
			HWND _hwnd;
			transform_chain _tc;

			virtual void move(int left, int top, int width, int height)
			{
				for (transform_chain::const_iterator i = _tc.begin(); i != _tc.end(); ++i)
					(*i)->unmap(left, top);
				::MoveWindow(_hwnd, left, top, width, height, TRUE);
			}

			virtual void set_parent(const transform_chain &tc, HWND parent)
			{
				_tc = tc;
				::SetParent(_hwnd, parent);
				::SetWindowLong(_hwnd, GWL_STYLE, (::GetWindowLong(_hwnd, GWL_STYLE) & ~WS_CHILD) | (parent ? WS_CHILD : 0));
			}

		public:
			TestView(shared_ptr<wpl::ui::widget> widget, HWND hwnd)
				: native_view(widget), _hwnd(hwnd)
			{	}
		};
	}

	TestNativeWidget::TestNativeWidget()
		: _hwnd(::CreateWindow(_T("static"), NULL, NULL, 0, 0, 50, 50, NULL, NULL, NULL, NULL))
	{	}

	TestNativeWidget::~TestNativeWidget()
	{	::DestroyWindow(_hwnd);	}

	shared_ptr<view> TestNativeWidget::create_custom_view()
	{	return shared_ptr<view>(new TestView(shared_from_this(), _hwnd));	}
}
