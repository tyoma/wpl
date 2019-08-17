#pragma once

#include <wpl/base/concepts.h>
#include <wpl/ui/view.h>
#include <wpl/ui/win32/native_view.h>

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace mocks
			{
				class native_view_window : public wpl::ui::native_view, noncopyable
				{
				public:
					native_view_window();
					~native_view_window();

					HWND hwnd() const throw();

				private:
					virtual HWND get_window(HWND hparent_for);

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
}
