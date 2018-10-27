#pragma once

#include <wpl/base/concepts.h>
#include <wpl/ui/view.h>
#include <wpl/ui/win32/native_view.h>
#include <wpl/ui/win32/types.h>

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace mocks
			{
				class native_control : public view, native_view, noncopyable
				{
				public:
					native_control();
					~native_control();

					virtual void resize(unsigned cx, unsigned cy, positioned_native_views &native_views);

					virtual HWND get_window() const throw();

				private:
					const HWND _hwnd;
				};
			}
		}
	}
}
