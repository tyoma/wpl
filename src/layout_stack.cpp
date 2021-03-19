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

#include <wpl/layout.h>

#include "helpers.h"

#include <agge/math.h>
#include <algorithm>
#include <numeric>
#include <wpl/cursor.h>
#include <wpl/drag_helper.h>
#include <wpl/helpers.h>
#include <wpl/static_visitor.h>
#include <wpl/view.h>

using namespace std;

namespace wpl
{
	namespace
	{
		struct static_size : unit_visitor<int>
		{
			int visit_pixel(double value) const {	return static_cast<int>(value);	}
			int visit_percent(double /*value*/) const {	return 0;	}
		};

		class calculate_size : public unit_visitor<int>, noncopyable
		{
		public:
			calculate_size(double remainder, double &correction)
				: _remainder(remainder), _correction(correction)
			{	}

			int visit_pixel(double value) const
			{	return agge::iround(static_cast<agge::real_t>(value));	}

			int visit_percent(double value) const
			{
				const auto fsize = agge::agge_max(0.01 * value * _remainder, 0.0);
				const auto ivalue = agge::iround(static_cast<agge::real_t>(fsize + _correction));

				_correction += fsize - ivalue;
				return ivalue;
			}

		private:
			const double _remainder;
			double &_correction;
		};

		class limit_lower : public unit_visitor<double>
		{
		public:
			limit_lower(double delta)
				: _delta(delta)
			{	}

			double visit_percent(double value) const {	return _delta < -value ? -value : _delta;	}

		private:
			double _delta;
		};

		class limit_upper : public unit_visitor<double>
		{
		public:
			limit_upper(double delta)
				: _delta(delta)
			{	}

			double visit_percent(double value) const {	return _delta > value ? value : _delta;	}

		private:
			double _delta;
		};

		class apply_delta : public unit_visitor<display_unit>
		{
		public:
			apply_delta(double delta)
				: _delta(delta)
			{	}

			display_unit visit_percent(double value) const {	return display_unit(value + _delta, display_unit::percent);	}

		private:
			double _delta;
		};
	}



	class stack::splitter : public view, wpl::noncopyable
	{
	public:
		splitter(stack &owner, size_t index, bool horizontal, shared_ptr<cursor_manager> cursor_manager_);

	private:
		virtual void mouse_enter() override;
		virtual void mouse_leave() override;
		virtual void mouse_move(int /*depressed*/, int x, int y) override;
		virtual void mouse_down(mouse_buttons button_, int /*depressed*/, int x, int y) override;
		virtual void mouse_up(mouse_buttons button_, int /*depressed*/, int /*x*/, int /*y*/) override;

	private:
		drag_helper _drag_helper;
		const shared_ptr<cursor_manager> _cursor_manager;
		stack &_owner;
		size_t _index;
		bool _horizontal;
	};



	int stack::item::min_size(bool horizontal, int for_opposite) const
	{	return horizontal ? child->min_width(for_opposite) : child->min_height(for_opposite);	}


	stack::splitter::splitter(stack &owner, size_t index, bool horizontal, shared_ptr<cursor_manager> cursor_manager_)
		: _cursor_manager(cursor_manager_), _owner(owner), _index(index), _horizontal(horizontal)
	{	}

	void stack::splitter::mouse_enter()
	{	_cursor_manager->push(_cursor_manager->get(_horizontal ? cursor_manager::h_resize : cursor_manager::v_resize));	}

	void stack::splitter::mouse_leave()
	{	_cursor_manager->pop();	}

	void stack::splitter::mouse_move(int /*depressed*/, int x, int y)
	{	_drag_helper.mouse_move(x, y);	}

	void stack::splitter::mouse_down(mouse_buttons button_, int /*depressed*/, int x, int y)
	{
		if (const auto delta = _owner.get_rsize())
		{
			_drag_helper.start([this, delta] (int dx, int dy) {
				_owner.move_splitter(_index, delta * (_horizontal ? dx : dy));
			}, capture, button_, x, y);
		}
	}

	void stack::splitter::mouse_up(mouse_buttons button_, int /*depressed*/, int /*x*/, int /*y*/)
	{	_drag_helper.mouse_up(button_);	}


	stack::stack(bool horizontal, shared_ptr<cursor_manager> cursor_manager_)
		: _cursor_manager(cursor_manager_), _spacing(0), _horizontal(horizontal)
	{	}

	void stack::set_spacing(int spacing)
	{
		_spacing = spacing;
		layout_changed(false);
	}

	void stack::add(shared_ptr<control> child, display_unit size, bool resizable, int tab_order)
	{
		item i = { child, size, resizable, tab_order };

		if (resizable && !_children.empty() && _children.back().resizable)
			_splitters.push_back(make_shared<splitter>(*this, _children.size() - 1, _horizontal, _cursor_manager));
		_children.push_back(i);
		container::add(*child);
	}

	void stack::layout(const placed_view_appender &append_view, const agge::box<int> &box)
	{
		auto location = box.w - box.w;	// '0' in coordinates type
		auto dynamic_space = (_horizontal ? box.w : box.h) - (static_cast<int>(_children.size()) - 1) * _spacing;
		auto splitter = _splitters.begin();
		auto correction = 0.0;
		const auto common_size = _horizontal ? box.h : box.w;
		const auto splitter_rect = create_rect(0, 0, _horizontal ? _spacing : box.w, _horizontal ? box.h : _spacing);

		for (auto i = _children.begin(); i != _children.end(); ++i)
			dynamic_space -= (max)(i->size.apply(static_size()), i->min_size(_horizontal, common_size));
		for (auto j = _children.begin(); j != _children.end(); location += _spacing)
		{
			const auto i = j++;
			const auto item_size = (max)(i->size.apply(calculate_size(dynamic_space, correction)),
				i->min_size(_horizontal, common_size));

			// Add child's views to layout.
			i->child->layout(offset(append_view, location, _horizontal, i->tab_order), create_box(item_size, box));
			location += item_size;

			// Add splitter view to layout, if needed and possible.
			if (i->resizable && (_children.end() != j && j->resizable))
			{
				placed_view pv = {	*splitter++, shared_ptr<native_view>(), splitter_rect, 0	};

				offset(append_view, location, _horizontal, 0)(pv);
			}
		}
		_last_size = dynamic_space;
	}

	int stack::min_height(int for_width) const
	{	return _horizontal ? 0 : min_shared(for_width);	}

	int stack::min_width(int for_height) const
	{	return _horizontal ? min_shared(for_height) : 0;	}

	agge::box<int> stack::create_box(int item_size, const agge::box<int> &self) const
	{	return _horizontal ? agge::create_box(item_size, self.h) : agge::create_box(self.w, item_size);	}

	double stack::get_rsize() const
	{	return _last_size ? 100.0 / _last_size : 0;	}

	void stack::move_splitter(size_t index, double delta)
	{
		delta = _children[index].size.apply(limit_lower(delta));
		delta = _children[index + 1].size.apply(limit_upper(delta));
		_children[index].size = _children[index].size.apply(apply_delta(delta));
		_children[index + 1].size = _children[index + 1].size.apply(apply_delta(-delta));
		layout_changed(false);
	}

	int stack::min_shared(int for_opposite) const
	{
		auto value = (static_cast<int>(_children.size()) - 1) * _spacing;

		for (auto i = _children.begin(); i != _children.end(); ++i)
			value += (max)(i->min_size(_horizontal, for_opposite), i->size.apply(static_size()));
		return value;
	}
}
