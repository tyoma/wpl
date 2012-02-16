#pragma once

#include <wpl/ui/win32/native_widget.h>

namespace ut
{
	struct NodesVisitationChecker : wpl::ui::node::visitor
	{
		virtual void visited(wpl::ui::widget &w)	{	visitation_log.push_back(std::make_pair(false, &w));	}
		virtual void visited(wpl::ui::container &c)	{	visitation_log.push_back(std::make_pair(true, &c));	}

		std::vector< std::pair<bool, void *> > visitation_log;
	};


	class TestNativeWidget : public wpl::ui::native_widget, wpl::noncopyable
	{
		HWND _hwnd;

		virtual std::shared_ptr<wpl::ui::container::widget_site> set_parent(HWND parent);

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
