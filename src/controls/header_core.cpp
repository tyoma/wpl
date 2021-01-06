//	Copyright (c) 2011-2021 by Artem A. Gevorkyan (gevorkyan.org)
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.

#include <wpl/controls/header_core.h>

#include <algorithm>
#include <wpl/cursor.h>
#include <wpl/helpers.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace controls
	{
		header_core::header_core(shared_ptr<cursor_manager> cursor_manager_)
			: _cursor_manager(cursor_manager_), _offset(0.0f), _ignore_invalidations(false)
		{	}

		header_core::~header_core()
		{	}

		void header_core::set_offset(double offset)
		{
			_offset = static_cast<agge::real_t>(offset);
			invalidate(nullptr);
		}

		void header_core::adjust_column_widths()
		{
			_ignore_invalidations = true;
			for (index_type i = 0, n = _model ? _model->get_count() : 0; i != n; ++i)
			{
				short current_width;
				const auto item_box = measure_item(*_model, i);

				_model->get_value(i, current_width);
				if (current_width < item_box.w)
					_model->update_column(i, static_cast<short>(item_box.w));
			}
			_ignore_invalidations = false;
		}

		int header_core::min_height(int /*for_width*/) const
		{
			int h = 0;

			for (index_type i = 0, n = _model ? _model->get_count() : 0; i != n; ++i)
				h = (max)(h, measure_item(*_model, i).h);
			return h;
		}

		void header_core::set_model(shared_ptr<columns_model> model)
		{
			if (model)
			{
				_model_invalidation = model->invalidate += [this] {
					if (!_ignore_invalidations)	// Untestable - the guard is for iteration rather than recursion.
						adjust_column_widths();
					invalidate(nullptr);
				};
				_model_sorting_change = model->sort_order_changed += [this] (index_type column, bool ascending) {
					_sorted_column = make_pair(column, ascending);
					invalidate(nullptr);
				};
				_sorted_column = model->get_sort_order();
			}
			else
			{
				_model_invalidation = nullptr;
				_model_sorting_change = nullptr;
			}
			_resize.cancel();
			_model = model;
			adjust_column_widths();
			invalidate(nullptr);
		}

		void header_core::mouse_enter()
		{	_cursor_manager->push(_cursor_manager->get(cursor_manager::arrow));	}

		void header_core::mouse_leave()
		{	_cursor_manager->pop();	}

		void header_core::mouse_move(int /*depressed*/, int x, int y)
		{
			if (!_resize.mouse_move(x, y))
			{
				const auto h = handle_from_point(x);

				_cursor_manager->set(_cursor_manager->get(h.second == resize_handle ? cursor_manager::h_resize
					: h.second == column_handle ? cursor_manager::hand : cursor_manager::arrow));
			}
		}

		void header_core::mouse_down(mouse_buttons button_, int /*depressed*/, int x, int y)
		{
			const auto h = handle_from_point(x);

			if (h.second == resize_handle)
			{
				const auto index = h.first;
				short initial_width;

				_model->get_value(h.first, initial_width);
				_resize.start([this, index, initial_width] (int dx, int) {
					auto w = (max<int>)(initial_width + dx, measure_item(*_model, index).w);

					_model->update_column(index, static_cast<short>(w));
				}, capture, button_, x, y);
			}
		}

		void header_core::mouse_up(mouse_buttons button_, int /*depressed*/, int x, int /*y*/)
		{
			if (!_resize.mouse_up(button_))
			{
				auto h = handle_from_point(x);

				if (h.second == column_handle)
					_model->activate_column(h.first);
			}
		}

		void header_core::draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer_) const
		{
			if (_model)
			{
				auto rc = create_rect(real_t(), real_t(), -_offset, get_last_size().h);

				for (index_type i = 0, n = _model->get_count(); i != n; ++i)
				{
					auto state = i == _sorted_column.first ? sorted | (_sorted_column.second ? ascending : 0) : 0;
					short width = 0;

					_model->get_value(i, width);
					rc.x1 = rc.x2;
					rc.x2 += width;

					draw_item(ctx, rasterizer_, rc, *_model, i, state);
				}
			}
		}

		box<int> header_core::measure_item(const columns_model &/*model*/, index_type /*index*/) const
		{	return zero();	}

		pair<header_core::index_type, header_core::handle_type> header_core::handle_from_point(int x) const
		{
			x += static_cast<int>(_offset);
			for (index_type i = 0, n = _model ? _model->get_count() : 0; x >= 0 && i != n; ++i)
			{
				short w;

				_model->get_value(i, w);
				if (x < w - 3)
					return make_pair(i, column_handle);
				if (x < w + 3)
					return make_pair(i, resize_handle);
				x -= w;
			}
			return make_pair(index_type(), none_handle);
		}
	}
}
