#pragma once

#include <wpl/concepts.h>
#include <wpl/view.h>
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

				HWND hwnd() const throw();

			private:
				virtual HWND materialize(HWND hparent);
				virtual LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam,
					const win32::window::original_handler_t &handler);

			private:
				HWND _hparent, _hwnd;
			};

			class native_view : public view
			{
			public:
				typedef std::vector< std::pair<std::shared_ptr<native_view_window>, view_location> > response_t;

			public:
				native_view();

				virtual void resize(unsigned cx, unsigned cy, visual::positioned_native_views &nviews);

			public:
				response_t response;
				bool container_was_dirty;
			};
		}
	}
}
