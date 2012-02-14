#pragma once

#include <wpl/ui/win32/native_widget.h>

namespace ut
{
	class TestNativeWidget : public wpl::ui::native_widget, wpl::noncopyable
	{
		HWND _hwnd;

		virtual void set_parent(HWND parent);

	public:
		TestNativeWidget();
		~TestNativeWidget();

		HWND hwnd() const;
	};


	class TestWidget : public wpl::ui::widget
	{
	};



	inline HWND TestNativeWidget::hwnd() const
	{	return _hwnd;	}
}
