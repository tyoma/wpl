#include "Mockups.h"

#include "helpers.h"

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
						virtual HWND get_window() const throw() { return 0; }
						virtual HWND get_window(HWND /*hparent_for*/) {	return 0;	}
					} g_dummy;
				}

				blender::blender()
					: has_uniform_color(true), _empty(true), _unicolor_set(false)
				{	}

				void blender::operator ()(const gcontext::pixel_type *buffer, int x, int y, agge::count_t count,
					const cover_type * /*covers*/) const
				{
					if (_empty)
					{
						min_x = x, max_x = x + count, max_y = min_y = y;
						_empty = false;
					}
					else
					{
						min_x = (min)(min_x, x);
						min_y = (min)(min_y, y);
						max_x = (max)(max_x, x + (int)count);
						max_y = (max)(max_y, y);
					}

					if (count)
					{
						if (!_unicolor_set)
						{
							uniform_color = *buffer;
							_unicolor_set = true;
						}

						for (; count-- && has_uniform_color; ++buffer)
							has_uniform_color = !memcmp(&uniform_color, buffer, sizeof(*buffer));
					}
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
