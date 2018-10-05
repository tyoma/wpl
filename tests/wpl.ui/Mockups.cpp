#include "Mockups.h"

#include "TestHelpers.h"

#include <wpl/ui/win32/native_view.h>

using namespace std;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace mocks
			{
				namespace
				{
					class nv : public native_view
					{
					public:
						~nv() {	}

					private:
						virtual HWND get_window() throw() { return 0; }
					} g_dummy;
				}

				void visual_with_native_view::resize(unsigned cx, unsigned cy, positioned_native_views &nviews)
				{	nviews.push_back(positioned_native_view(g_dummy, make_position(0, 0, cx, cy)));	}

				void logging_layout_manager::layout(unsigned width, unsigned height, container::positioned_view *widgets, size_t count) const
				{
					reposition_log.push_back(make_pair(width, height));
					last_widgets.assign(widgets, widgets + count);

					for (vector<view_location>::const_iterator i = positions.begin(); count && i != positions.end();
						--count, ++widgets, ++i)
					{
						widgets->location = *i;
					}
				}


				void fill_layout::layout(unsigned width, unsigned height, container::positioned_view *widgets, size_t count) const
				{
					for (; count; --count, ++widgets)
					{
						widgets->location.left = 0;
						widgets->location.top = 0;
						widgets->location.width = width;
						widgets->location.height = height;
					}
				}
			}
		}
	}
}
