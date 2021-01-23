#pragma once

#include <algorithm>
#include <vector>
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
				control();

				virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) override;
				virtual int min_height(int for_width) const override;
				virtual int min_width(int for_height) const override;

			public:
				std::vector< agge::box<int> > size_log;
				std::vector<placed_view> views;
				mutable std::vector<int> for_width_log, for_height_log;
				int minimum_width, minimum_height;
			};



			inline control::control()
				: minimum_width(0), minimum_height(0)
			{	}

			inline void control::layout(const placed_view_appender &append_view, const agge::box<int> &box)
			{
				size_log.push_back(box);
				std::for_each(views.begin(), views.end(), append_view);
			}

			inline int control::min_height(int for_width) const
			{	return for_width_log.push_back(for_width), minimum_height;	}

			inline int control::min_width(int for_height) const
			{	return for_height_log.push_back(for_height), minimum_width;	}
		}
	}
}
