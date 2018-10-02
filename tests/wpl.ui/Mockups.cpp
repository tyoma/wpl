#include "Mockups.h"

using namespace std;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace mocks
			{
				void logging_layout_manager::layout(unsigned width, unsigned height, container::positioned_view *widgets, size_t count) const
				{
					reposition_log.push_back(make_pair(width, height));
					last_widgets.assign(widgets, widgets + count);

					for (vector<position>::const_iterator i = positions.begin(); count && i != positions.end();
						--count, ++widgets, ++i)
					{
						widgets->left = i->left;
						widgets->top = i->top;
						widgets->width = i->width;
						widgets->height = i->height;
					}
				}


				void fill_layout::layout(unsigned width, unsigned height, container::positioned_view *widgets, size_t count) const
				{
					for (; count; --count, ++widgets)
					{
						widgets->left = 0;
						widgets->top = 0;
						widgets->width = width;
						widgets->height = height;
					}
				}
			}
		}
	}
}
