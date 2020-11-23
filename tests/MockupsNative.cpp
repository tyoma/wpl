#include "MockupsNative.h"

#include <tchar.h>
#include <windows.h>
#include <wpl/win32/native_view.h>
#include <ut/assert.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace mocks
		{
			native_view_window::native_view_window()
				: native_view(""), _hparent(0)
			{	}

			native_view_window::~native_view_window()
			{	::DestroyWindow(_hwnd);	}

			HWND native_view_window::materialize(HWND hparent_for)
			{
				assert_not_null(hparent_for);
				assert_is_true(!_hparent || hparent_for == _hparent);

				if (!_hparent)
				{
					_hwnd = ::CreateWindow(_T("static"), NULL, WS_CHILD, 0, 0, 0, 0, hparent_for, NULL, NULL, 0);
					_hparent = hparent_for;
				}
				return _hwnd;
			}

			LRESULT native_view_window::on_message(UINT message, WPARAM wparam, LPARAM lparam,
				const win32::window::original_handler_t &handler)
			{	return handler(message, wparam, lparam);	}

			HWND native_view_window::hwnd() const throw()
			{	return _hwnd;	}


			native_view::native_view()
				: container_was_dirty(false)
			{	}

			void native_view::resize(unsigned /*cx*/, unsigned /*cy*/, positioned_native_views &nviews)
			{
				container_was_dirty = container_was_dirty || !nviews.empty();
				for (response_t::const_iterator i = response.begin(); i != response.end(); ++i)
					nviews.push_back(visual::positioned_native_view(*i->first, i->second));
			}
		}
	}
}
