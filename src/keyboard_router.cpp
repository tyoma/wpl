//	Copyright (c) 2011-2022 by Artem A. Gevorkyan (gevorkyan.org)
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

#include <wpl/keyboard_router.h>

#include <iterator>
#include <wpl/view.h>

using namespace std;

namespace wpl
{
	namespace
	{
		struct tab_order_less
		{
			bool operator ()(const placed_view &lhs, const placed_view &rhs) const
			{	return lhs.tab_order < rhs.tab_order;	}
		};

		template <typename ContainerT, typename IteratorT>
		IteratorT cycle_next(ContainerT &container, IteratorT i)
		{	return ++i == container.end() ? container.begin() : i;	}

		template <typename ContainerT, typename IteratorT>
		IteratorT cycle_previous(ContainerT &container, IteratorT i)
		{	return i = (i == container.begin() ? container.end() : i), --i;	}

		void got_focus(keyboard_router_host &host, const placed_view &pv)
		{
			if (pv.regular)
				pv.regular->got_focus();
			else
				host.set_focus(*pv.native);
		}

		void lost_focus(const placed_view &pv)
		{
			if (pv.regular)
				pv.regular->lost_focus();
		}
	}

	keyboard_router::keyboard_router(const vector<placed_view> &views, keyboard_router_host &host)
		: _views(views), _host(host), _has_focus(false), _focus(end(_ordered))
	{	}

	void keyboard_router::reload_views()
	{
		_ordered.clear();
		copy_if(_views.begin(), _views.end(), back_inserter(_ordered), [] (const placed_view &pv) {
			return !!pv.tab_order && (pv.regular || pv.native);
		});
		stable_sort(begin(_ordered), end(_ordered), tab_order_less());
		_focus = begin(_ordered);
		if (_has_focus && _focus != end(_ordered))
			wpl::got_focus(_host, *_focus);
	}

	void keyboard_router::set_focus(const keyboard_input *input)
	{
		const auto new_focus = find_if(begin(_ordered), end(_ordered), [input] (const placed_view &pv) {
			return pv.regular.get() == input;
		});

		if (end(_ordered) != new_focus)
			move_focus(new_focus);
	}

	void keyboard_router::key_down(unsigned code, int m)
	{
		if (end(_ordered) == _focus)
			return;

		if (keyboard_input::tab == code)
		{
			auto new_focus = m & keyboard_input::shift ? cycle_previous(_ordered, _focus) : cycle_next(_ordered, _focus);

			if (new_focus != _focus)
				move_focus(new_focus);
		}
		else if (_focus->regular)
		{
			_focus->regular->key_down(code, m);
		}
	}

	void keyboard_router::character(wchar_t /*symbol*/, unsigned /*repeats*/, int /*modifiers*/)
	{	}

	void keyboard_router::key_up(unsigned code, int modifiers)
	{
		if (end(_ordered) == _focus)
			return;

		if (keyboard_input::tab != code && _focus->regular)
			_focus->regular->key_up(code, modifiers);
	}

	void keyboard_router::got_focus()
	{
		_has_focus = true;
		if (end(_ordered) != _focus && _focus->regular)
			_focus->regular->got_focus();
	}

	void keyboard_router::lost_focus()
	{
		_has_focus = false;
		if (end(_ordered) != _focus && _focus->regular)
			_focus->regular->lost_focus();
	}

	void keyboard_router::move_focus(placed_views::const_iterator to)
	{
		const auto current = _focus;

		if (current == to)
			return;
		else if (to->native)
			_host.set_focus(*to->native);
		else if (current->native)
			_focus = to, _host.set_focus();
		else
			current->regular->lost_focus(), to->regular->got_focus(), _focus = to;
	}
}
