//	Copyright (c) 2011-2020 by Artem A. Gevorkyan (gevorkyan.org)
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

#include <wpl/cursor.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace controls
	{
		struct header_core::resize_data
		{
			index_type index;
			int click_width;
			std::shared_ptr<void> capture_handle;
		};


		header_core::header_core(shared_ptr<cursor_manager> cursor_manager_)
			: _cursor_manager(cursor_manager_), _offset(0.0f)
		{	}

		header_core::~header_core()
		{	}

		void header_core::set_offset(double offset)
		{
			_offset = static_cast<agge::real_t>(offset);
			invalidate(nullptr);
		}

		void header_core::set_model(shared_ptr<columns_model> model)
		{
			if (model)
			{
				_model_invalidation = model->invalidated += [this] { invalidate(nullptr); };
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
			_model = model;
			_resize.reset();
			invalidate(nullptr);
		}

		void header_core::mouse_enter()
		{	_cursor_manager->push(_cursor_manager->get(cursor_manager::arrow));	}

		void header_core::mouse_leave()
		{	_cursor_manager->pop();	}

		void header_core::mouse_move(int /*depressed*/, int x, int /*y*/)
		{
			const auto h = handle_from_point(x);
			const auto c = h.second == resize_handle ? cursor_manager::h_resize
				: h.second == column_handle ? cursor_manager::hand : cursor_manager::arrow;

			_cursor_manager->set(_cursor_manager->get(c));
			if (_resize)
			{
				_model->update_column(_resize->index, static_cast<short>((max<int>)(_resize->click_width + x,
					measure_column(*_model, _resize->index))));
			}
		}

		void header_core::mouse_down(mouse_buttons /*button*/, int /*depressed*/, int x, int /*y*/)
		{
			const auto h = handle_from_point(x);

			if (h.second == resize_handle)
			{
				short w;
				resize_data resize = { h.first, -x, };

				capture(resize.capture_handle);
				_model->get_value(h.first, w);
				resize.click_width += w;
				_resize.reset(new resize_data(resize));
			}
		}

		void header_core::mouse_up(mouse_buttons /*button*/, int /*depressed*/, int x, int /*y*/)
		{
			if (_resize)
			{
				_resize.reset();
			}
			else
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
				rect_r rc = { 0.0f, 0.0f, -_offset, _size.h };
				columns_model::column c;

				for (index_type i = 0, n = _model->get_count(); i != n; ++i)
				{
					auto state = i == _sorted_column.first ? sorted | (_sorted_column.second ? ascending : 0) : 0;

					_model->get_column(i, c);
					rc.x1 = rc.x2;
					rc.x2 += c.width;

					draw_item_background(ctx, rasterizer_, rc, i, state);
					draw_item(ctx, rasterizer_, rc, i, state, c.caption );
				}
			}
		}

		void header_core::layout(const placed_view_appender &append_view, const agge::box<int> &box_)
		{
			_size.h = static_cast<real_t>(box_.h);
			integrated_control<wpl::header>::layout(append_view, box_);
		}

		short header_core::measure_column(columns_model &/*model*/, index_type /*index*/) const
		{	return 0;	}

		void header_core::draw_item_background(gcontext &/*ctx*/, gcontext::rasterizer_ptr &/*rasterizer*/,
			const rect_r &/*box*/, index_type /*item*/, unsigned /*item_state_flags*/ /*state*/) const
		{	}

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
