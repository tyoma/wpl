#pragma once

#include <wpl/ui/widget.h>
#include <wpl/ui/layout.h>

namespace std
{
   using tr1::weak_ptr;
}

typedef struct HWND__ *HWND;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace mocks
			{
				class logging_layout_manager : public wpl::ui::layout_manager
				{
				public:
					mutable std::vector< std::pair<size_t, size_t> > reposition_log;
					mutable std::vector<widget_position> last_widgets;
					std::vector<position> positions;

				private:
					virtual void layout(unsigned width, unsigned height, widget_position *widgets, size_t count) const;
				};


				class fill_layout : public wpl::ui::layout_manager
				{
				private:
					virtual void layout(unsigned width, unsigned height, widget_position *widgets, size_t count) const;
				};


				class TestNativeWidget
					: public wpl::ui::widget, wpl::noncopyable
				{
					HWND _hwnd;

				public:
					TestNativeWidget();
					~TestNativeWidget();

					virtual std::shared_ptr<wpl::ui::view> create_view(const wpl::ui::native_root &r);

					HWND hwnd() const;

					std::vector< std::weak_ptr<wpl::ui::view> > views_created;
				};



				inline HWND TestNativeWidget::hwnd() const
				{	return _hwnd;	}
			}
		}
	}
}
