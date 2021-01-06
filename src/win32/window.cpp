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

#include <wpl/win32/window.h>

#include <stdexcept>

using namespace std;
using namespace std::placeholders;

namespace wpl
{
	namespace win32
	{
		namespace
		{
			class tls_base
			{
			protected:
				tls_base()
					: _index(::TlsAlloc())
				{	}

				~tls_base()
				{	::TlsFree(_index);	}

				void *get() const
				{	return ::TlsGetValue(_index);	}

				void set(void *value)
				{	::TlsSetValue(_index, value); }

			private:
				unsigned _index;
			};

			size_t knuth_hash(unsigned int key) throw()
			{	return key * 2654435761;	}

			size_t knuth_hash(unsigned long long int key) throw()
			{	return static_cast<size_t>(key * 0x7FFFFFFFFFFFFFFF);	}

			WNDPROC set_wndproc(HWND hwnd, WNDPROC value)
			{
				return reinterpret_cast<WNDPROC>(::SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(value)));
			}

			WNDPROC get_wndproc(HWND hwnd)
			{	return reinterpret_cast<WNDPROC>(::GetWindowLongPtr(hwnd, GWLP_WNDPROC));	}
		}


		template <typename T>
		struct window::tls : private tls_base
		{
			T *get() const
			{	return static_cast<T *>(tls_base::get());	}

			void set(T *value)
			{	return tls_base::set(value);	}
		};


		shared_ptr< window::tls< unordered_map<HWND, window *, window::hwnd_hash> > >
			window::_windows_s(new window::tls< unordered_map<HWND, window *, window::hwnd_hash> >);


		size_t window::hwnd_hash::operator ()(HWND hwnd) const
		{	return knuth_hash(reinterpret_cast<size_t>(hwnd));	}


		window::window(HWND hwnd, const user_handler_t &user_handler)
			: _hwnd(hwnd), _wndproc(0), _user_handler(user_handler), _windows(_windows_s)
		{	}

		shared_ptr<window> window::attach(HWND hwnd, const user_handler_t &user_handler)
		{
			if (::IsWindow(hwnd))
			{
				auto_ptr<window> w(new window(hwnd, user_handler));

				w->map();
				return shared_ptr<window>(w.release(), bind(&window::detach, _1));
			}
			throw invalid_argument("");
		}

		HWND window::hwnd() const throw()
		{	return _hwnd;	}

		LRESULT window::operator ()(UINT message, WPARAM wparam, LPARAM lparam) const
		{	return ::CallWindowProc(_wndproc, _hwnd, message, wparam, lparam);	}

		LRESULT CALLBACK window::windowproc_proxy(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
		{
			window *w = get_window(hwnd);

			if (WM_NCDESTROY == message)
				w->unmap(true);

			LRESULT result = w->_user_handler ? w->_user_handler(message, wparam, lparam, *w) : (*w)(message, wparam, lparam);

			if (WM_NCDESTROY == message)
			{
				w->_hwnd = 0;
				if (!w->_user_handler)
					delete w;
			}
			return result;
		}

		void window::map()
		{
			if (windows_map *m = _windows->get())
			{
				if (get_window(_hwnd))
					throw logic_error("A window is already subclassed!");
				m->insert(make_pair(_hwnd, this));
			}
			else
			{
				auto_ptr<windows_map> mm(new windows_map);

				mm->insert(make_pair(_hwnd, this));
				_windows->set(mm.release());
			}
			_wndproc = set_wndproc(_hwnd, &windowproc_proxy);
		}

		window *window::get_window(HWND hwnd) throw()
		{
			if (windows_map *m = _windows_s->get())
			{
				windows_map::const_iterator i = m->find(hwnd);

				if (i != m->end())
					return i->second;
			}
			return 0;
		}

		bool window::unmap(bool force) throw()
		{
			if (windows_map *m = _windows->get())
			{
				const windows_map::const_iterator i = m->find(_hwnd);

				if (i != m->end() && (force || &windowproc_proxy == get_wndproc(i->second->_hwnd)))
				{
					set_wndproc(_hwnd, i->second->_wndproc);
					m->erase(i);
					if (m->empty())
					{
						_windows->set(0);
						delete m;
					}
					return true;
				}
			}
			return false;
		}

		void window::detach()
		{
			_user_handler = window::user_handler_t();
			if (!_hwnd || unmap(false))
				delete this;
		}
	}
}
