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
				void logging_layout_manager::layout(unsigned width, unsigned height, view_position *widgets, size_t count) const
				{
					reposition_log.push_back(make_pair(width, height));
					last_widgets.assign(widgets, widgets + count);

					for (vector<position>::const_iterator i = positions.begin(); count && i != positions.end();
						--count, ++widgets, ++i)
						widgets->second = *i;
				}


				void fill_layout::layout(unsigned width, unsigned height, view_position *widgets, size_t count) const
				{
					const position p = { 0, 0, static_cast<int>(width), static_cast<int>(height) };

					for (; count; --count, ++widgets)
						widgets->second = p;
				}
			}
		}
	}
}
