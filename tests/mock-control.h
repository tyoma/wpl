#pragma once

#include <algorithm>
#include <wpl/control.h>

namespace wpl
{
	namespace tests
	{
		namespace mocks
		{
			class control : public wpl::control
			{
			public:
				virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) override;

			public:
				std::vector< agge::box<int> > size_log;
				std::vector<placed_view> views;
			};



			inline void control::layout(const placed_view_appender &append_view, const agge::box<int> &box)
			{
				size_log.push_back(box);
				std::for_each(views.begin(), views.end(), append_view);
			}
		}
	}
}
