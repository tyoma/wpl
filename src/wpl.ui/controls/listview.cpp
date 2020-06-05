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

#include <wpl/ui/controls/listview.h>

#include <algorithm>
#include <cmath>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace ui
	{
		namespace controls
		{
			struct listview_core::vertical_scroll_model : scroll_model
			{
				virtual pair<double /*range_min*/, double /*range_width*/> get_range() const
				{
					if (!owner || !owner->_model)
						return make_pair(0, 0);
					return make_pair(0, static_cast<double>(owner->_model->get_count()));
				}

				virtual pair<double /*window_min*/, double /*window_width*/> get_window() const
				{
					if (!owner || !owner->get_item_height())
						return make_pair(0, 0);
					return make_pair(owner->_first_visible, owner->_size.h / owner->get_item_height());
				}

				virtual void scrolling(bool /*begins*/)
				{	}

				virtual void scroll_window(double window_min, double /*window_width*/)
				{
					if (!owner)
						return;
					owner->_first_visible = window_min;
					owner->invalidate(0);
					invalidated();
				}

				listview_core *owner;
			};


			listview_core::listview_core()
				: _first_visible(0), _vsmodel(new vertical_scroll_model), _focus_item(npos())
			{
				_size.w = 0, _size.h = 0;
				_vsmodel->owner = this;
			}

			listview_core::~listview_core()
			{	_vsmodel->owner = 0;	}

			shared_ptr<scroll_model> listview_core::get_vscroll_model()
			{	return _vsmodel;	}

			void listview_core::key_down(unsigned code, int modifiers)
			{
				if (down == code)
				{
					_focus_item++;
				}
				else if (up == code)
				{
					if (npos() != _focus_item)
						_focus_item--;
				}
				if (!(control & modifiers))
					select(_focus_item, true);
			}

			void listview_core::mouse_down(mouse_buttons /*button*/, int /*buttons*/, int /*x*/, int y)
			{
				const real_t item_height = get_item_height();

				_focus_item = static_cast<index_type>((static_cast<real_t>(y) + 0.5f) / item_height);
				select(_focus_item, true);
			}

			void listview_core::mouse_up(mouse_buttons /*button*/, int /*buttons*/, int /*x*/, int /*y*/)
			{	}

			void listview_core::draw(gcontext &ctx, gcontext::rasterizer_ptr &ras) const
			{
				if (!_model | !_cmodel)
					return;

				const index_type rows = _model->get_count();
				const columns_model::index_type columns = _cmodel->get_count();
				const real_t item_height = get_item_height();
				real_t total_width = 0.0f;
				index_type r = (max)(0, static_cast<int>(_first_visible));
				rect_r box = { 0.0f, item_height * static_cast<real_t>(r - _first_visible), 0.0f, 0.0f };

				_widths.clear();
				for (columns_model::index_type c = 0; c != columns; ++c)
				{
					_cmodel->get_column(c, _column_buffer);
					_widths.push_back(static_cast<real_t>(_column_buffer.width));
					total_width += static_cast<real_t>(_column_buffer.width);
				}
				for (; box.y2 = box.y1 + item_height, r != rows && box.y1 < _size.h; ++r, box.y1 = box.y2)
				{
					const unsigned state = (_selected_items.count(r) ? selected : 0) | (_focus_item == r ? focused : 0);

					box.x1 = 0.0f, box.x2 = total_width;
					draw_item_background(ctx, ras, box, r, state);
					for (columns_model::index_type c = 0; c != columns; box.x1 = box.x2, ++c)
					{
						box.x2 = box.x1 + _widths[c];
						draw_subitem_background(ctx, ras, box, r, state, c);
					}
					box.x1 = 0.0f, box.x2 = total_width;
					draw_item(ctx, ras, box, r, state);
					for (columns_model::index_type c = 0; c != columns; box.x1 = box.x2, ++c)
					{
						_model->get_text(r, c, _text_buffer);
						box.x2 = box.x1 + _widths[c];
						draw_subitem(ctx, ras, box, r, state, c, _text_buffer);
					}
				}
			}

			void listview_core::resize(unsigned cx, unsigned cy, positioned_native_views &/*native_views*/)
			{
				_size.w = static_cast<real_t>(cx);
				_size.h = static_cast<real_t>(cy);
				invalidate(0);
				_vsmodel->invalidated();
			}

			void listview_core::set_columns_model(shared_ptr<columns_model> cmodel)
			{	_cmodel = cmodel;	}

			void listview_core::set_model(shared_ptr<table_model> model)
			{
				_model_invalidation = model ? model->invalidated += [this] (index_type count) {
					if (_item_count != count)
						_vsmodel->invalidated(), _item_count = count;
					invalidate(0);
				} : slot_connection();
				_item_count = model ? model->get_count() : 0u;
				_model = model;
			}

			void listview_core::adjust_column_widths()
			{	}

			void listview_core::select(index_type item, bool reset_previous)
			{
				if (reset_previous)
					_selected_items.clear();
				_selected_items.insert(item);
				invalidate(0);
			}

			void listview_core::clear_selection()
			{	}

			void listview_core::ensure_visible(index_type /*item*/)
			{	}

			void listview_core::draw_subitem_background(gcontext &/*ctx*/, gcontext::rasterizer_ptr &/*rasterizer*/,
				const agge::rect_r &/*box*/, index_type /*item*/, unsigned /*state*/, index_type /*subitem*/) const
			{	}

			void listview_core::draw_item(gcontext &/*ctx*/, gcontext::rasterizer_ptr &/*rasterizer*/,
				const agge::rect_r &/*box*/, index_type /*item*/, unsigned /*state*/) const
			{	}
		}
	}
}
