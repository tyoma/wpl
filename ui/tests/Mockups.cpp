#include "Mockups.h"

#include <wpl/ui/win32/native_view.h>

#include <tchar.h>
#include <windows.h>

using namespace std;

namespace ut
{
	namespace
	{
		class TestView : public wpl::ui::view
		{
			TestWidget &_test_widget;

		public:
			TestView(shared_ptr<wpl::ui::widget> widget, TestWidget &test_widget)
				: view(widget), _test_widget(test_widget)
			{	}

			virtual void move(int left, int top, int width, int height)
			{
				wpl::ui::layout_manager::position p = {	left, top, width, height	};

				_test_widget.reposition_log.push_back(p);
			}
		};
	}


	void logging_layout_manager::layout(size_t width, size_t height, widget_position *widgets, size_t count) const
	{
		reposition_log.push_back(make_pair(width, height));
		last_widgets.assign(widgets, widgets + count);

		for (vector<position>::const_iterator i = positions.begin(); count && i != positions.end();
			--count, ++widgets, ++i)
			widgets->second = *i;
	}


	TestNativeWidget::TestNativeWidget()
		: _hwnd(::CreateWindow(_T("static"), NULL, NULL, 0, 0, 50, 50, NULL, NULL, NULL, NULL))
	{	}

	TestNativeWidget::~TestNativeWidget()
	{	::DestroyWindow(_hwnd);	}

	shared_ptr<wpl::ui::view> TestNativeWidget::create_view(const wpl::ui::native_root &r)
	{
		shared_ptr<wpl::ui::view> v(new wpl::ui::native_view(shared_from_this(), _hwnd, r));

		views_created.push_back(v);
		return v;
	}

	shared_ptr<wpl::ui::view> TestWidget::create_view(const wpl::ui::native_root &/*r*/)
	{
		shared_ptr<wpl::ui::view> v(new TestView(shared_from_this(), *this));

		views_created.push_back(v);
		return v;
	}
}
