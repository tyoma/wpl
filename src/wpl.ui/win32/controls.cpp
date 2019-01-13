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
			class text_container_impl : public BaseT
			{
				virtual void set_text(const wstring &text)
				{	::SetWindowTextW(get_window(), text.c_str());	}
			};


			class button_impl : public text_container_impl< native_view<button> >
			{
			public:
				button_impl();

			private:
				virtual LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam,
					const window::original_handler_t &handler);
			};


			class link_impl : public text_container_impl< native_view<link> >
			{
			public:
				link_impl();

			private:
				virtual LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam,
					const window::original_handler_t &handler);
			};



			button_impl::button_impl()
			{
				init(::CreateWindow(WC_BUTTON, NULL, WS_POPUP, 0, 0, 100, 100, NULL, NULL, NULL, NULL), true);
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
			{
				init(::CreateWindow(_T("SysLink"), NULL, LWS_RIGHT | WS_POPUP, 0, 0, 100, 100, NULL, NULL, NULL,
					NULL), true);
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
		}

		shared_ptr<button> create_button()
		{	return shared_ptr<button>(new win32::button_impl);	}

		shared_ptr<link> create_link()
		{	return shared_ptr<link>(new win32::link_impl);	}
	}
}
