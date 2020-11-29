#include "Mockups.h"

#include "helpers-visual.h"

#include <string.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace mocks
		{
			namespace
			{
				native_view *g_dummy = 0;
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


			cursor_manager::cursor_manager()
				: stack_level(0u)
			{	}

			shared_ptr<const cursor> cursor_manager::get(standard_cursor id) const
			{
				const auto i = cursors.find(id);

				assert_not_equal(cursors.end(), i);
				return i->second;
			}

			void cursor_manager::set(shared_ptr<const cursor> cursor_)
			{	recently_set = cursor_;	}

			void cursor_manager::push(shared_ptr<const cursor> cursor_)
			{
				stack_level++;
				recently_set = cursor_;
			}

			void cursor_manager::pop()
			{	stack_level--;	}
		}
	}
}
