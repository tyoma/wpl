#pragma once

#include <wpl/concepts.h>
#include <wpl/win32/native_view.h>

namespace wpl
{
	namespace tests
	{
		namespace mocks
		{
			class native_view_window : public wpl::native_view, noncopyable
			{
			public:
				native_view_window();
				~native_view_window();

			private:
				virtual HWND materialize(HWND hparent) override;
				virtual LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam,
					const win32::window::original_handler_t &handler) override;

			private:
				HWND _hparent, _hwnd;
			};
		}
	}
}
