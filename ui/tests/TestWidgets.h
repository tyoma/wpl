#pragma once

#include <wpl/ui/widget.h>

typedef struct HWND__ *HWND;

namespace ut
{
	struct position
	{
		int left, top;
		int width, height;
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
		std::vector<position> reposition_log;
	};



	inline HWND TestNativeWidget::hwnd() const
	{	return _hwnd;	}
}
