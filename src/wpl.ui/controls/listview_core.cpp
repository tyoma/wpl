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

#include <algorithm>
#include <cmath>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace controls
	{
		struct listview_core::trackable_less
		{
			bool operator ()(const trackable_ptr &lhs, const trackable_ptr &rhs)
			{	return lhs->index() < rhs->index();	}

			bool operator ()(const trackable_ptr &lhs, index_type rhs)
			{	return lhs->index() < rhs;	}

			bool operator ()(index_type lhs, const trackable_ptr &rhs)
			{	return lhs < rhs->index();	}
		};


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

			virtual void scrolling(bool begins)
			{
				if (owner)
					owner->_state_vscrolling = begins, owner->_state_keep_focus_visible = false;
			}

			virtual void scroll_window(double window_min, double /*window_width*/)
			{
				if (!owner)
					return;
				owner->_first_visible = window_min;
				owner->invalidate_();
				invalidated();
			}

			listview_core *owner;
		};

		struct listview_core::horizontal_scroll_model : scroll_model
		{
			virtual pair<double /*range_min*/, double /*range_width*/> get_range() const
			{	return make_pair(0, 0);	}

			virtual pair<double /*window_min*/, double /*window_width*/> get_window() const
			{	return make_pair(0, 0);	}

			virtual void scrolling(bool /*begins*/)
			{	}

			virtual void scroll_window(double /*window_min*/, double /*window_width*/)
			{	}
		};


		listview_core::listview_core()
			: _first_visible(0), _vsmodel(new vertical_scroll_model), _hsmodel(new horizontal_scroll_model),
				_state_vscrolling(false)
		{
			_size.w = 0, _size.h = 0;
			_vsmodel->owner = this;
		}

		listview_core::~listview_core()
		{	_vsmodel->owner = nullptr;	}

		shared_ptr<scroll_model> listview_core::get_vscroll_model()
		{	return _vsmodel;	}

		shared_ptr<scroll_model> listview_core::get_hscroll_model()
		{	return _hsmodel;	}

		void listview_core::focus(index_type item)
		{
			if (!_model)
				return;
			_focused = item != npos() ? _model->track(item) : nullptr;
			_state_keep_focus_visible = true;
			invalidate_();
			make_visible(item);
		}

		void listview_core::make_visible(index_type item)
		{
			if (is_visible(item) || _state_vscrolling)
				return;
			_first_visible = static_cast<double>(item < _first_visible ? item : item - _size.h / get_item_height() + 1);
			_vsmodel->invalidated();
			invalidate_();
		}

		void listview_core::key_down(unsigned code, int modifiers)
		{
			index_type item = _focused ? _focused->index() : npos();

			switch (code)
			{
			case down:
				if (npos() == item)
					item = 0;
				else if (item + 1 < _model->get_count())
					item++;
				break;

			case up:
				if (npos() != item && item)
					item--;
				break;
			}

			focus(item);
			if (!(keyboard_input::control & modifiers))
				select(item, true);
		}

		void listview_core::mouse_down(mouse_buttons /*button*/, int buttons, int /*x*/, int y)
		{
			if (!(buttons & keyboard_input::control))
			{
				const index_type item = get_item(y);

				focus(item);
				select(item, true);
			}
		}

		void listview_core::mouse_up(mouse_buttons /*button*/, int buttons, int /*x*/, int y)
		{
			if (buttons & keyboard_input::control)
			{
				const index_type item = get_item(y);

				focus(item);
				toggle_selection(item);
			}
		}

		void listview_core::draw(gcontext &ctx, gcontext::rasterizer_ptr &ras) const
		{
			if (!_model | !_cmodel)
				return;

			const index_type rows = _model->get_count();
			const columns_model::index_type columns = _cmodel->get_count();
			const real_t item_height = get_item_height();
			const index_type focused_item = _focused ? _focused->index() : npos();
			real_t total_width = 0.0f;
			index_type r = (max)(0, static_cast<int>(_first_visible));
			rect_r box = { 0.0f, item_height * static_cast<real_t>(r - _first_visible), 0.0f, 0.0f };

			_widths.clear();
			for (columns_model::index_type c = 0; c != columns; ++c)
			{
				short int width;
				_cmodel->get_value(c, width);
				_widths.push_back(width);
				total_width += width;
			}
			for (; box.y2 = box.y1 + item_height, r != rows && box.y1 < _size.h; ++r, box.y1 = box.y2)
			{
				const unsigned state = (is_selected(r) ? selected : 0) | (focused_item == r ? focused : 0);

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
			invalidate_();
			_vsmodel->invalidated();
		}

		shared_ptr<view> listview_core::get_view()
		{	return shared_from_this();	}

		void listview_core::set_columns_model(shared_ptr<columns_model> cmodel)
		{
			_cmodel_invalidation = cmodel ? cmodel->invalidated += [this] {	invalidate_();	} : nullptr;
			_cmodel = cmodel;
		}

		void listview_core::set_model(shared_ptr<table_model> model)
		{
			if (model == _model)
				return;
			_model_invalidation = model ? model->invalidated += [this] (index_type count) {
				if (_item_count != count)
					_vsmodel->invalidated(), _item_count = count;
				sort(_selected.begin(), _selected.end(), listview_core::trackable_less());
				if (_focused && _state_keep_focus_visible)
					make_visible(_focused->index());
				invalidate_();
			} : nullptr;
			_item_count = model ? model->get_count() : 0u;
			_model = model;
			_focused = nullptr;
			_selected.clear();
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

		void listview_core::ensure_visible(index_type /*item*/)
		{	}

		void listview_core::draw_subitem_background(gcontext &/*ctx*/, gcontext::rasterizer_ptr &/*rasterizer*/,
			const agge::rect_r &/*box*/, index_type /*item*/, unsigned /*state*/, index_type /*subitem*/) const
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

		listview_core::index_type listview_core::get_item(int y) const
		{
			const double item_height = get_item_height();

			return static_cast<index_type>((y + _first_visible * item_height + 0.5) / item_height);
		}

		bool listview_core::is_selected(index_type item) const
		{	return binary_search(_selected.begin(), _selected.end(), item, trackable_less());	}

		bool listview_core::is_visible(index_type item) const
		{
			const real_t tolerance = 0.001f;
			const real_t item_height = get_item_height();
			const real_t lower = item_height * static_cast<real_t>(item - _first_visible), upper = lower + item_height;

			return (-tolerance < lower) & (upper < _size.h + tolerance);
		}
	}
}
