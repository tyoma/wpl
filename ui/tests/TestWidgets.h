#pragma once

#include <wpl/ui/widget.h>

typedef struct HWND__ *HWND;

namespace ut
{
	struct NodesVisitationChecker : wpl::ui::node::visitor
	{
		virtual void visited(wpl::ui::widget &w)	{	visitation_log.push_back(std::make_pair(false, &w));	}
		virtual void visited(wpl::ui::container &c)	{	visitation_log.push_back(std::make_pair(true, &c));	}

		std::vector< std::pair<bool, void *> > visitation_log;
	};


	struct ViewVisitationChecker : wpl::ui::view::visitor
	{
		virtual void generic_view_visited(wpl::ui::view &v)	{	visitation_log.push_back(std::make_pair(false, &v));	}
		virtual void native_view_visited(wpl::ui::native_view &v)	{	visitation_log.push_back(std::make_pair(true, &v));	}

		std::vector< std::pair<bool, void *> > visitation_log;
	};

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

		virtual std::shared_ptr<wpl::ui::view> create_view();

		HWND hwnd() const;
	};


	class TestWidget : public wpl::ui::widget
	{
	public:
		virtual std::shared_ptr<wpl::ui::view> create_view();

		std::vector< std::weak_ptr<wpl::ui::view> > views_created;
		std::vector<position> reposition_log;
	};



	inline HWND TestNativeWidget::hwnd() const
	{	return _hwnd;	}
}
