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

#include <agge/math.h>
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
		placed_view_appender offset(const placed_view_appender &inner, int d, bool horizontal, int tab_override)
		{
			// TODO: use custom appender functor, as function may allocate storage dynamically.

			return [&inner, d, horizontal, tab_override] (placed_view pv) {
				wpl::offset(pv.location, horizontal ? d : 0, horizontal ? 0 : d);
				pv.tab_order = pv.tab_order ? tab_override ? tab_override : pv.tab_order : 0;
				inner(pv);
			};
		}
	}

	class resizable_stack::splitter : public view, wpl::noncopyable
	{
	public:
		splitter(resizable_stack &owner, size_t index, bool horizontal, shared_ptr<cursor_manager> cursor_manager_)
			: _cursor_manager(cursor_manager_), _owner(owner), _index(index), _horizontal(horizontal)
		{	}

	private:
		virtual void mouse_enter() override
		{
			_cursor_manager->push(_cursor_manager->get(_horizontal ? cursor_manager::h_resize : cursor_manager::v_resize));
		}

		virtual void mouse_leave() override
		{	_cursor_manager->pop();	}

		virtual void mouse_move(int /*depressed*/, int x, int y) override
		{	_drag_helper.mouse_move(x, y);	}

		virtual void mouse_down(mouse_buttons button_, int /*depressed*/, int x, int y) override
		{
			if (const auto delta = _owner.get_step())
			{
				_drag_helper.start([this, delta] (int dx, int dy) {
					_owner.move_splitter(_index, delta * (_horizontal ? dx : dy));
				}, capture, button_, x, y);
			}
		}

		virtual void mouse_up(mouse_buttons button_, int /*depressed*/, int /*x*/, int /*y*/) override
		{	_drag_helper.mouse_up(button_);	}

	private:
		drag_helper _drag_helper;
		const shared_ptr<cursor_manager> _cursor_manager;
		resizable_stack &_owner;
		size_t _index;
		bool _horizontal;
	};

	resizable_stack::resizable_stack(int spacing, bool horizontal, shared_ptr<cursor_manager> cursor_manager_)
		: _cursor_manager(cursor_manager_), _spacing(spacing), _horizontal(horizontal)
	{	}

	void resizable_stack::add(shared_ptr<control> child, double size_fraction, int tab_order)
	{
		item i = { child, size_fraction, tab_order };

		if (!_children.empty())
			_splitters.push_back(make_shared<splitter>(*this, _children.size() - 1, _horizontal, _cursor_manager));
		_children.push_back(i);
	}

	void resizable_stack::layout(const placed_view_appender &append_view, const agge::box<int> &box)
	{
		const auto children_space = (_horizontal ? box.w : box.h) - static_cast<int>(_children.size() - 1) * _spacing;
		const auto rsum = 1 / accumulate(_children.begin(), _children.end(), 0.0, [] (double acc, const item &i) {
			return acc + i.size_fraction;
		});

		auto b = box;
		auto location = box.w - box.w; // '0' in coordinates type
		auto correction = 0.0;
		auto splitter = _splitters.begin();
		const auto calculate_item_size = [children_space, &correction, rsum] (const item &i) -> int {
			const auto fsize = agge::agge_max(i.size_fraction * children_space * rsum, 0.0);
			const auto isize = agge::iround(static_cast<agge::real_t>(fsize + correction));

			correction += fsize - isize;
			return isize;
		};

		for (auto i = _children.begin(); i != _children.end(); ++i)
		{
			auto &item_size = (_horizontal ? b.w : b.h);

			item_size = calculate_item_size(*i);
			if (_children.begin() != i)
			{
				placed_view pv = {	*splitter++, shared_ptr<native_view>(), create_rect(0, 0, box.w, box.h), 0	};

				(_horizontal ? pv.location.x2 : pv.location.y2) = _spacing;
				offset(append_view, location, _horizontal, 0)(pv);
				location += _spacing;
			}
			i->child->layout(offset(append_view, location, _horizontal, i->tab_order), b);
			location += item_size;
		}
		_last_size = children_space;
	}

	double resizable_stack::get_step() const
	{
		return _last_size ? accumulate(_children.begin(), _children.end(), 0.0, [] (double acc, const item &i) {
			return acc + i.size_fraction;
		}) / _last_size : 0;
	}

	void resizable_stack::move_splitter(size_t index, double delta)
	{
		if (delta < -_children[index].size_fraction)
			delta = -_children[index].size_fraction;
		else if (delta > _children[index + 1].size_fraction)
			delta = _children[index + 1].size_fraction;
		_children[index].size_fraction += delta;
		_children[index + 1].size_fraction -= delta;
		layout_changed(false);
	}
}
