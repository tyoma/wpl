//	Copyright (c) 2011-2021 by Artem A. Gevorkyan (gevorkyan.org)
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.

#pragma once

#include "../concepts.h"
#include "../view.h"

#include <memory>

namespace wpl
{
	struct placed_view;

	namespace controls
	{
		template <typename ControlT>
		class integrated_control : public ControlT, public view, public std::enable_shared_from_this<view>, noncopyable
		{
		public:
			integrated_control();

			// control methods
			virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) override;

		protected:
			const agge::box_r &get_last_size() const;

		protected:
			bool tab_stop;

		private:
			agge::box_r _last_size;
		};



		template <typename ControlT>
		inline integrated_control<ControlT>::integrated_control()
			: tab_stop(false), _last_size(agge::zero())
		{	}

		template <typename ControlT>
		inline void integrated_control<ControlT>::layout(const placed_view_appender &append_view, const agge::box<int> &box_)
		{
			placed_view v = {
				shared_from_this(),
				std::shared_ptr<native_view>(),
				{ 0, 0, box_.w, box_.h },
				!!tab_stop,
				false
			};

			_last_size.w = static_cast<agge::real_t>(box_.w), _last_size.h = static_cast<agge::real_t>(box_.h);
			append_view(v);
		}

		template <typename ControlT>
		inline const agge::box_r &integrated_control<ControlT>::get_last_size() const
		{	return _last_size;	}
	}
}
