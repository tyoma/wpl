//	Copyright (c) 2011-2018 by Artem A. Gevorkyan (gevorkyan.org)
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

#include <wpl/ui/win32/window.h>

#include <wpl/base/concepts.h>

#include <stdexcept>

using namespace std;

namespace wpl
{
	namespace ui
	{
		namespace
		{
			size_t knuth_hash(unsigned int key) throw()
			{	return key * 2654435761;	}

			size_t knuth_hash(unsigned long long int key) throw()
			{	return static_cast<size_t>(key * 0x7FFFFFFFFFFFFFFF);	}
		}

		mt::tls< unordered_map<HWND, window *, window::hwnd_hash> > window::_windows;


		size_t window::hwnd_hash::operator ()(HWND hwnd) const
		{	return knuth_hash(reinterpret_cast<size_t>(hwnd));	}


		window::window(HWND hwnd, const user_handler_t &user_handler)
			: _hwnd(hwnd), _wndproc(0), _user_handler(user_handler)
		{	}

		shared_ptr<window> window::attach(HWND hwnd, const user_handler_t &user_handler)
		{
			if (::IsWindow(hwnd))
			{
				auto_ptr<window> w(new window(hwnd, user_handler));

				map(hwnd, w.get());
				return shared_ptr<window>(w.release(), &window::detach);
			}
			throw invalid_argument("");
		}

		HWND window::hwnd() const throw()
		{	return _hwnd;	}

		LRESULT window::operator ()(UINT message, WPARAM wparam, LPARAM lparam) const
		{	return ::CallWindowProc(reinterpret_cast<WNDPROC>(_wndproc), _hwnd, message, wparam, lparam);	}

		LRESULT CALLBACK window::windowproc_proxy(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
		{
			window *w = get_window(hwnd);

			if (WM_NCDESTROY == message)
				unmap(hwnd, true);

			LRESULT result = w->_user_handler ? w->_user_handler(message, wparam, lparam, *w) : (*w)(message, wparam, lparam);

			if (WM_NCDESTROY == message)
			{
				w->_hwnd = 0;
				if (!w->_user_handler)
					delete w;
			}
			return result;
		}

		void window::map(HWND hwnd, window *w)
		{
			if (windows_map *m = _windows.get())
			{
				if (get_window(hwnd))
					throw logic_error("A window is already subclassed!");
				m->insert(make_pair(hwnd, w));
			}
			else
			{
				auto_ptr<windows_map> mm(new windows_map);

				mm->insert(make_pair(hwnd, w));
				_windows.set(mm.release());
			}
			w->_wndproc = ::SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&windowproc_proxy));
		}

		window *window::get_window(HWND hwnd) throw()
		{
			if (windows_map *m = _windows.get())
			{
				windows_map::const_iterator i = m->find(hwnd);

				if (i != m->end())
					return i->second;
			}
			return 0;
		}

		bool window::unmap(HWND hwnd, bool f) throw()
		{
			windows_map *m = _windows.get();
			windows_map::const_iterator i = m->find(hwnd);

			if (!f && &windowproc_proxy != reinterpret_cast<WNDPROC>(::GetWindowLongPtr(i->second->_hwnd, GWLP_WNDPROC)))
				return false;
			::SetWindowLongPtr(hwnd, GWLP_WNDPROC, i->second->_wndproc);
			m->erase(i);
			if (m->empty())
			{
				_windows.set(0);
				delete m;
			}
			return true;
		}

		void window::detach(window *w)
		{
			w->_user_handler = window::user_handler_t();
			if (!w->_hwnd || unmap(w->_hwnd, false))
				delete w;
		}
	}
}
