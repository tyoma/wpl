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

#include <wpl/ui/controls.h>

#include "native_view.h"

#include <commctrl.h>
#include <olectl.h>
#include <tchar.h>

using namespace std;

namespace wpl
{
	namespace ui
	{
		namespace win32
		{
			template <typename BaseT>
			class text_container_impl : public BaseT, public native_view<view>
			{
				virtual void set_text(const wstring &text)
				{
					_text = text;
					::SetWindowTextW(get_window(), _text.c_str());
				}

			protected:
				typedef text_container_impl text_container;

			protected:
				virtual void set_align(ui::text_container::halign value)
				{	_halign = value;	}

			protected:
				wstring _text;
				ui::text_container::halign _halign;
			};


			class button_impl : public text_container_impl<button>, public enable_shared_from_this<button_impl>
			{
			public:
				button_impl();

			private:
				virtual shared_ptr<view> get_view();

				virtual HWND materialize(HWND hparent);
				virtual LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam,
					const window::original_handler_t &handler);
			};


			class link_impl : public text_container_impl<link>, public enable_shared_from_this<link_impl>
			{
			public:
				link_impl();

			private:
				virtual shared_ptr<view> get_view();

				virtual HWND materialize(HWND hparent);
				virtual LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam,
					const window::original_handler_t &handler);

				virtual void set_align(halign value);
			};



			button_impl::button_impl()
			{	}

			shared_ptr<view> button_impl::get_view()
			{	return shared_from_this();	}

			HWND button_impl::materialize(HWND hparent)
			{
				return ::CreateWindow(WC_BUTTON, _text.c_str(), WS_CHILD | WS_VISIBLE, 0, 0, 100, 100, hparent, NULL, NULL,
					NULL);
			}

			LRESULT button_impl::on_message(UINT message, WPARAM wparam, LPARAM lparam,
				const window::original_handler_t &handler)
			{
				switch (message)
				{
				default:
					return handler(message, wparam, lparam);

				case OCM_COMMAND:
					if (HIWORD(wparam) == BN_CLICKED)
						clicked();
					return 0;
				}
			}


			link_impl::link_impl()
			{	_halign = ui::text_container::left;	}

			shared_ptr<view> link_impl::get_view()
			{	return shared_from_this();	}

			HWND link_impl::materialize(HWND hparent)
			{
				return ::CreateWindow(WC_LINK, _text.c_str(), WS_CHILD | WS_VISIBLE
					| (ui::text_container::right == _halign ? LWS_RIGHT : 0), 0, 0, 100, 100, hparent, NULL, NULL, NULL);
			}

			LRESULT link_impl::on_message(UINT message, WPARAM wparam, LPARAM lparam,
				const window::original_handler_t &handler)
			{
				switch (message)
				{
				default:
					return handler(message, wparam, lparam);

				case OCM_NOTIFY:
					const NMLINK *nmlink = reinterpret_cast<const NMLINK *>(lparam);

					if (NM_CLICK == nmlink->hdr.code)
						clicked(nmlink->item.iLink, nmlink->item.szUrl);
					return 0;
				}
			}

			void link_impl::set_align(halign value)
			{
				text_container::set_align(value);
				switch (value)
				{
				case ui::text_container::left:
					SetWindowLong(get_window(), GWL_STYLE, ~LWS_RIGHT & GetWindowLong(get_window(), GWL_STYLE));
					break;

				case ui::text_container::right:
					SetWindowLong(get_window(), GWL_STYLE, LWS_RIGHT | GetWindowLong(get_window(), GWL_STYLE));
					break;
				}
			}
		}

		shared_ptr<button> create_button()
		{	return shared_ptr<button>(new win32::button_impl);	}

		shared_ptr<link> create_link()
		{	return shared_ptr<link>(new win32::link_impl);	}
	}
}
