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

#include <wpl/controls/listview_core.h>

#include <agge/math.h>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <wpl/helpers.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace controls
	{
		namespace
		{
			const real_t c_tolerance = 0.001f;

		}

		struct listview_core::trackable_less
		{
			bool operator ()(const trackable_ptr &lhs, const trackable_ptr &rhs)
			{	return lhs->index() < rhs->index();	}

			bool operator ()(const trackable_ptr &lhs, index_type rhs)
			{	return lhs->index() < rhs;	}

			bool operator ()(index_type lhs, const trackable_ptr &rhs)
			{	return lhs < rhs->index();	}
		};

		struct listview_core::base_scroll_model : scroll_model
		{
			typedef pair<double, double> range;

			base_scroll_model()
				: owner(nullptr)
			{	}

			virtual range get_range() const override
			{	return make_pair(0, owner ? get_max() : 0);	}

			virtual void scroll_window(double window_min, double /*window_width*/) override
			{
				if (!owner)
					return;
				set_window(window_min);
				owner->invalidate_();
				invalidate();
			}

			virtual double get_max() const = 0;
			virtual void set_window(double window_min) = 0;

			listview_core *owner;
		};

		struct listview_core::vertical_scroll_model : base_scroll_model
		{
			virtual range get_window() const override
			{
				return owner && owner->get_minimal_item_height()
					? range(owner->_offset.dy, owner->get_last_size().h / owner->get_minimal_item_height()) : range(0, 0);
			}

			virtual double get_increment() const override
			{	return 1;	}

			virtual void scrolling(bool begins) override
			{
				if (owner)
					owner->_state_vscrolling = begins, owner->_state_keep_focus_visible = false;
			}

			virtual double get_max() const override
			{	return owner->_model ? static_cast<double>(owner->_model->get_count()) : 0;	}

			virtual void set_window(double window_min) override
			{	owner->_offset.dy = window_min;	}
		};

		struct listview_core::horizontal_scroll_model : base_scroll_model
		{
			virtual range get_window() const override
			{	return owner ? range(owner->_offset.dx, owner->get_last_size().w) : range(0, 0);	}

			virtual double get_increment() const override
			{	return 5;	}

			virtual void scrolling(bool /*begins*/) override
			{	}

			virtual double get_max() const override
			{
				double total = 0;

				if (const auto cmodel = owner->_cmodel)
				{
					for (columns_model::index_type i = 0, count = cmodel->get_count(); i != count; ++i)
					{
						short w = 0;

						cmodel->get_value(i, w);
						total += w;
					}
				}
				return total;
			}

			virtual void set_window(double window_min) override
			{	owner->_offset.dx = window_min;	}
		};


		listview_core::listview_core()
			: _vsmodel(new vertical_scroll_model), _hsmodel(new horizontal_scroll_model), _state_vscrolling(false)
		{
			tab_stop = true;
			_offset.dx = 0, _offset.dy = 0;
			_vsmodel->owner = this;
			_hsmodel->owner = this;
		}

		listview_core::~listview_core()
		{
			_vsmodel->owner = nullptr;
			_hsmodel->owner = nullptr;
		}

		shared_ptr<scroll_model> listview_core::get_vscroll_model()
		{	return _vsmodel;	}

		shared_ptr<scroll_model> listview_core::get_hscroll_model()
		{	return _hsmodel;	}

		void listview_core::make_visible(index_type item)
		{
			if (is_visible(item) | _state_vscrolling | (npos() == item))
				return;
			_offset.dy = static_cast<double>(item < _offset.dy ? item : item - get_last_size().h / get_minimal_item_height() + 1);
			_vsmodel->invalidate();
			invalidate_();
		}

		void listview_core::key_down(unsigned code, int modifiers)
		{
			if (const index_type item_count = _model ? _model->get_count() : index_type())
			{
				index_type focused = _focused ? _focused->index() : npos();
				const index_type scroll_size = agge::iround(get_last_size().h / get_minimal_item_height());
				const index_type last = item_count - 1;
				const index_type first_visible = first_partially_visible();
				const index_type last_visible = last_partially_visible();

				switch (code)
				{
				case up:
					if (npos() != focused && focused)
						focused--;
					break;

				case page_up:
					if (npos() == focused)
						focused = first_visible;
					else if (npos() == first_visible)
						focused = 0;
					else if (focused > first_visible)
						focused = first_visible;
					else if (focused > scroll_size)
						focused -= scroll_size;
					else
						focused = 0;
					break;

				case down:
					if (npos() == focused)
						focused = 0;
					else if (focused < last)
						focused++;
					break;

				case page_down:
					if (npos() == focused)
						focused = last_visible;
					else if (npos() == last_visible)
						focused = last;
					else if (focused < last_visible)
						focused = last_visible;
					else if (focused + scroll_size < item_count)
						focused += scroll_size;
					else
						focused = last;
					break;
				}

				focus(focused);
				if (!(keyboard_input::control & modifiers))
					select(focused, true);
			}
		}

		void listview_core::mouse_down(mouse_buttons /*button*/, int depressed, int /*x*/, int y)
		{
			if (!(depressed & keyboard_input::control))
			{
				const index_type item = get_item(y);

				focus(item);
				select(item, true);
			}
		}

		void listview_core::mouse_up(mouse_buttons /*button*/, int depressed, int /*x*/, int y)
		{
			if (depressed & keyboard_input::control)
			{
				const index_type item = get_item(y);

				focus(item);
				toggle_selection(item);
			}
		}

		void listview_core::mouse_double_click(mouse_buttons /*button*/, int /*depressed*/, int /*x*/, int y)
		{
			const index_type item = get_item(y);

			if (item != npos())
				item_activate(item);
		}

		void listview_core::draw(gcontext &ctx, gcontext::rasterizer_ptr &ras) const
		{
			if (!_model | !_cmodel)
				return;

			const auto &size = get_last_size();
			const index_type item_count = _model->get_count();
			const columns_model::index_type columns = _cmodel->get_count();
			const real_t item_height = get_minimal_item_height();
			const index_type focused_item = has_focus && _focused ? _focused->index() : npos();
			real_t total_width = 0.0f;
			index_type r = (max)(0, static_cast<int>(_offset.dy));
			auto box = create_rect(real_t(), item_height * static_cast<real_t>(r - _offset.dy), real_t(), real_t());

			_widths.clear();
			for (columns_model::index_type c = 0; c != columns; ++c)
			{
				short int width;

				_cmodel->get_value(c, width);
				_widths.push_back(width);
				total_width += width;
			}
			for (; box.y2 = box.y1 + item_height, r < item_count && box.y1 < size.h; ++r, box.y1 = box.y2)
			{
				const unsigned state = (is_selected(r) ? selected : 0) | (focused_item == r ? focused : 0);

				box.x1 = -static_cast<agge::real_t>(_offset.dx), box.x2 = box.x1 + total_width;
				draw_item_background(ctx, ras, box, r, state);
				for (columns_model::index_type c = 0; c != columns; box.x1 = box.x2, ++c)
				{
					box.x2 = box.x1 + _widths[c];
					if (box.x2 < 0.0 || box.x1 >= size.w - c_tolerance)
						continue;
					draw_subitem_background(ctx, ras, box, r, state, c);
				}
				box.x1 = -static_cast<agge::real_t>(_offset.dx), box.x2 = box.x1 + total_width;
				draw_item(ctx, ras, box, r, state);
				for (columns_model::index_type c = 0; c != columns; box.x1 = box.x2, ++c)
				{
					box.x2 = box.x1 + _widths[c];
					if (box.x2 < 0.0 || box.x1 >= size.w - c_tolerance)
						continue;
					_model->get_text(r, c, _text_buffer);
					draw_subitem(ctx, ras, box, r, state, c, _text_buffer);
				}
			}
		}

		void listview_core::layout(const placed_view_appender &append_view, const agge::box<int> &box_)
		{
			integrated_control<wpl::listview>::layout(append_view, box_);
			_vsmodel->invalidate();
			_hsmodel->invalidate();
		}

		void listview_core::set_columns_model(shared_ptr<columns_model> cmodel)
		{
			_cmodel_invalidation = cmodel ? cmodel->invalidate += [this] {
				invalidate_();
				_hsmodel->invalidate();
			} : nullptr;
			_cmodel = cmodel;
			_hsmodel->invalidate();
		}

		void listview_core::set_model(shared_ptr<table_model> model)
		{
			if (model == _model)
				return;
			_model_invalidation = model ? model->invalidate += [this] (index_type count) {
				if (_item_count != count)
					_vsmodel->invalidate(), _item_count = count;
				sort(_selected.begin(), _selected.end(), listview_core::trackable_less());
				if (_focused && _state_keep_focus_visible)
					make_visible(_focused->index());
				invalidate_();
			} : nullptr;
			_item_count = model ? model->get_count() : 0u;
			_model = model;
			_focused = nullptr;
			_selected.clear();
			invalidate_();
		}

		void listview_core::adjust_column_widths()
		{	}

		void listview_core::select(index_type item, bool reset_previous)
		{
			if (npos() == item && !reset_previous)
				return;
			if (reset_previous)
			{
				for (auto i = _selected.begin(); i != _selected.end(); ++i)
					selection_changed((*i)->index(), false);
				_selected.clear();
			}
			if (const trackable_ptr t = npos() != item ? _model->track(item) : nullptr)
			{
				_selected.insert(upper_bound(_selected.begin(), _selected.end(), t, trackable_less()), t);
				selection_changed(item, true);
			}
			invalidate_();
		}

		void listview_core::focus(index_type item)
		{
			if (!_model)
				return;
			_focused = item != npos() ? _model->track(item) : nullptr;
			_state_keep_focus_visible = true;
			make_visible(item);
			invalidate_();
		}

		void listview_core::draw_subitem_background(gcontext &/*ctx*/, gcontext::rasterizer_ptr &/*rasterizer*/,
			const agge::rect_r &/*box*/, index_type /*item*/, unsigned /*state*/,
			columns_model::index_type /*subitem*/) const
		{	}

		void listview_core::draw_item(gcontext &/*ctx*/, gcontext::rasterizer_ptr &/*rasterizer*/,
			const agge::rect_r &/*box*/, index_type /*item*/, unsigned /*state*/) const
		{	}

		void listview_core::invalidate_()
		{	visual::invalidate(nullptr);	}

		void listview_core::toggle_selection(index_type item)
		{
			trackables::iterator i = lower_bound(_selected.begin(), _selected.end(), item, trackable_less());

			if (i != _selected.end() && (*i)->index() == item)
				_selected.erase(i), selection_changed(item, false);
			else
				select(item, false);
		}

		listview_core::index_type listview_core::first_partially_visible() const
		{	return _offset.dy >= 0.0f ? static_cast<index_type>(_offset.dy) : npos();	}

		listview_core::index_type listview_core::last_partially_visible() const
		{	return get_item(static_cast<int>(get_last_size().h - 1));	}

		listview_core::index_type listview_core::get_item(int y) const
		{
			const auto item_height = get_minimal_item_height();
			const auto item = (y + _offset.dy * item_height + 0.5) / item_height;

			return _model && 0 <= item && item < _model->get_count() ? static_cast<index_type>(item) : table_model::npos();
		}

		bool listview_core::is_selected(index_type item) const
		{	return binary_search(_selected.begin(), _selected.end(), item, trackable_less());	}

		bool listview_core::is_visible(index_type item) const
		{
			const real_t item_height = get_minimal_item_height();
			const real_t lower = item_height * static_cast<real_t>(item - _offset.dy), upper = lower + item_height;

			return (-c_tolerance < lower) & (upper < get_last_size().h + c_tolerance);
		}
	}
}
