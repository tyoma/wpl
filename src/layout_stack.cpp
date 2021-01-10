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
#include <wpl/view.h>

using namespace std;

namespace wpl
{
	namespace
	{
		class accumulator
		{
		public:
			accumulator(double remainder)
				: _remainder(remainder)
			{	}

			void visit_pixel(double value)
			{	_remainder -= value;	}

			void visit_percent(double /*value*/)
			{	}

			void visit_em(double /*value*/)
			{	}

			double get_remainder() const
			{	return _remainder;	}

		private:
			double _remainder;
		};

		class calculate_size : noncopyable
		{
		public:
			calculate_size(int &value, double remainder)
				: _value(value), _remainder(remainder), _correction(0)
			{	}

			void visit_pixel(double value)
			{	_value = agge::iround(static_cast<agge::real_t>(value));	}

			void visit_percent(double value)
			{
				const auto fsize = agge::agge_max(0.01 * value * _remainder, 0.0);

				_value = agge::iround(static_cast<agge::real_t>(fsize + _correction));
				_correction += fsize - _value;
			}

			void visit_em(double /*value*/)
			{	}

		private:
			int &_value;
			double _remainder, _correction;
		};

		class limit_delta : noncopyable
		{
		public:
			limit_delta(double &delta, bool lower)
				: _delta(delta), _lower(lower)
			{	}

			void visit_pixel(double /*value*/) const
			{	}

			void visit_percent(double value) const
			{
				if (_lower && _delta < -value)
					_delta = -value;
				else if (!_lower && _delta > value)
					_delta = value;
			}

			void visit_em(double /*value*/) const
			{	}

		private:
			double &_delta;
			const bool _lower;
		};

		struct apply_delta : noncopyable
		{
			apply_delta(double delta_)
				: delta(delta_)
			{	}

			void visit_pixel(double /*value_*/) const {	}
			void visit_percent(double &value_) const {	value_ += delta;	}
			void visit_em(double /*value*/) const {	}

			double delta;
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


	stack::stack(int spacing, bool horizontal, shared_ptr<cursor_manager> cursor_manager_)
		: _cursor_manager(cursor_manager_), _spacing(spacing), _horizontal(horizontal)
	{	}

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
		auto b = box;
		auto location = b.w - b.w;	// '0' in coordinates type
		const auto children_space = (_horizontal ? box.w : box.h) - (static_cast<int>(_children.size()) - 1) * _spacing;
		accumulator acc(children_space);
		auto splitter = _splitters.begin();

		for_each(_children.begin(), _children.end(), [&] (const item &i) {	i.size.apply(acc);	});

		auto &item_size = _horizontal ? b.w : b.h;
		calculate_size item_size_calc(_horizontal ? b.w : b.h, acc.get_remainder());

		for (auto i = _children.begin(); i != _children.end(); )
		{
			const auto current = i++;
			const auto last = _children.end() == i;

			// Add child's views to layout.
			current->size.apply(item_size_calc);
			current->child->layout(offset(append_view, location, _horizontal, current->tab_order), b);
			location += item_size;

			// Add splitter view to layout, if possible.
			if (!last && current->resizable && i->resizable)
			{
				placed_view pv = {	*splitter++, shared_ptr<native_view>(), create_rect(0, 0, box.w, box.h), 0	};

				(_horizontal ? pv.location.x2 : pv.location.y2) = _spacing;
				offset(append_view, location, _horizontal, 0)(pv);
			}
			location += _spacing;
		}
		_last_size = children_space;
	}

	double stack::get_rsize() const
	{	return _last_size ? 100.0 / _last_size : 0;	}

	void stack::move_splitter(size_t index, double delta)
	{
		_children[index].size.apply<const limit_delta>(limit_delta(delta, true));
		_children[index + 1].size.apply<const limit_delta>(limit_delta(delta, false));
		_children[index].size.apply<const apply_delta>(apply_delta(delta));
		_children[index + 1].size.apply<const apply_delta>(apply_delta(-delta));
		layout_changed(false);
	}
}
