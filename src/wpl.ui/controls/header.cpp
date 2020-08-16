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

#include <wpl/controls/header.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace controls
	{
		header::header()
			: _offset(0.0f), _resizing_colum(make_pair(npos(), 0))
		{	}

		void header::set_model(const shared_ptr<columns_model> &model)
		{
			_model = model;
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
			}
		}

		void header::set_offset(double offset)
		{
			_offset = static_cast<agge::real_t>(offset);
			invalidate(nullptr);
		}

		void header::mouse_move(int /*depressed*/, int x, int /*y*/)
		{
			if (_resizing_colum.first != npos())
				_model->update_column(_resizing_colum.first, static_cast<short>(_resizing_colum.second + x));
		}

		void header::mouse_down(mouse_buttons /*button*/, int /*buttons*/, int x, int /*y*/)
		{
			pair<index_type, handle_type> h = handle_from_point(x);

			if (h.second == resize_handle)
			{
				short w;

				_model->get_value(h.first, w);
				_resizing_colum = make_pair(h.first, w - x);
				capture(_resizing_capture);
			}
		}

		void header::mouse_up(mouse_buttons /*button*/, int /*buttons*/, int x, int /*y*/)
		{
			if (_resizing_colum.first != npos())
			{
				_resizing_colum.first = npos();
				_resizing_capture.reset();
			}
			else
			{
				pair<index_type, handle_type> h = handle_from_point(x);

				if (h.second == column_handle)
					_model->activate_column(h.first);
			}
		}

		void header::draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer_) const
		{
			if (_model)
			{
				rect_r rc = { 0.0f, 0.0f, -_offset, _size.h };
				columns_model::column c;

				for (index_type i = 0, n = _model->get_count(); i != n; ++i)
				{
					unsigned int state = i == _sorted_column.first ? sorted | (_sorted_column.second ? ascending : 0) : 0;

					_model->get_column(i, c);
					rc.x1 = rc.x2;
					rc.x2 += c.width;

					draw_item_background(ctx, rasterizer_, rc, i, state);
					draw_item(ctx, rasterizer_, rc, i, state, c.caption );
				}
			}
		}

		void header::resize(unsigned /*cx*/, unsigned cy, positioned_native_views &/*native_views*/)
		{	_size.h = static_cast<real_t>(cy);	}

		void header::draw_item_background(gcontext &/*ctx*/, gcontext::rasterizer_ptr &/*rasterizer*/,
			const rect_r &/*box*/, index_type /*item*/, unsigned /*item_state_flags*/ /*state*/) const
		{	}

		pair<header::index_type, header::handle_type> header::handle_from_point(int x) const
		{
			x += static_cast<int>(_offset);
			for (index_type i = 0, n = _model ? _model->get_count() : 0; i != n; ++i)
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
