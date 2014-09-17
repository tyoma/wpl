#pragma once

#include <wpl/ui/widget.h>
#include <wpl/ui/layout.h>

namespace std
{
   using tr1::weak_ptr;
}

typedef struct HWND__ *HWND;

namespace ut
{
	class logging_layout_manager : public wpl::ui::layout_manager
	{
	public:
		mutable std::vector< std::pair<size_t, size_t> > reposition_log;
		mutable std::vector<widget_position> last_widgets;
		std::vector<position> positions;

	private:
		virtual void layout(size_t width, size_t height, widget_position *widgets, size_t count) const;
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


	class TestWidget : public wpl::ui::widget
	{
	public:
		virtual std::shared_ptr<wpl::ui::view> create_view(const wpl::ui::native_root &r);

		std::vector< std::weak_ptr<wpl::ui::view> > views_created;
		std::vector<wpl::ui::layout_manager::position> reposition_log;
	};



	inline HWND TestNativeWidget::hwnd() const
	{	return _hwnd;	}
}
