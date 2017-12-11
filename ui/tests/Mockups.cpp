#include "Mockups.h"

#include <wpl/ui/win32/native_view.h>

#include <tchar.h>
#include <windows.h>

using namespace std;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace mocks
			{
				void logging_layout_manager::layout(unsigned width, unsigned height, widget_position *widgets, size_t count) const
				{
					reposition_log.push_back(make_pair(width, height));
					last_widgets.assign(widgets, widgets + count);

					for (vector<position>::const_iterator i = positions.begin(); count && i != positions.end();
						--count, ++widgets, ++i)
						widgets->second = *i;
				}


				void fill_layout::layout(unsigned width, unsigned height, widget_position *widgets, size_t count) const
				{
					const position p = { 0, 0, static_cast<int>(width), static_cast<int>(height) };

					for (; count; --count, ++widgets)
						widgets->second = p;
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
			}
		}
	}
}
