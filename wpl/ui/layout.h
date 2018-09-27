//	Copyright (c) 2011-2018 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "view.h"

#include "../base/concepts.h"

namespace wpl
{
	namespace ui
	{
		struct layout_manager
		{
			struct position;
			
			typedef std::pair<std::shared_ptr<const view>, position> view_position;

			virtual ~layout_manager() {	}

			virtual void layout(unsigned width, unsigned height, view_position *views, size_t count) const = 0;
		};

		struct layout_manager::position
		{
			int left, top, width, height;
		};


		template <int layout_manager::position::*SharedPosition, int layout_manager::position::*SharedSize,
			int layout_manager::position::*CommonPosition, int layout_manager::position::*CommonSize>
		class stack : noncopyable
		{
		public:
			template <typename InputIterator>
			stack(InputIterator begin, InputIterator end, unsigned spacing);

			void layout(unsigned shared_size, unsigned common_size, layout_manager::view_position * const widgets,
				const size_t count) const;

		protected:
			typedef stack base;

		private:
			const std::vector<int> _sizes;
			const unsigned _spacing;
		};


		class hstack : public layout_manager, stack<&layout_manager::position::left, &layout_manager::position::width,
			&layout_manager::position::top, &layout_manager::position::height>
		{
		public:
			template <typename InputIterator>
			hstack(InputIterator begin, InputIterator end, unsigned spacing);

			virtual void layout(unsigned width, unsigned height, view_position *widgets, size_t count) const;
		};


		class vstack : public layout_manager, stack<&layout_manager::position::top, &layout_manager::position::height,
			&layout_manager::position::left, &layout_manager::position::width>
		{
		public:
			template <typename InputIterator>
			vstack(InputIterator begin, InputIterator end, unsigned spacing);

			virtual void layout(unsigned width, unsigned height, view_position *widgets, size_t count) const;
		};



		template <int layout_manager::position::*SharedPosition, int layout_manager::position::*SharedSize,
			int layout_manager::position::*CommonPosition, int layout_manager::position::*CommonSize>
		template <typename InputIterator>
		inline stack<SharedPosition, SharedSize, CommonPosition, CommonSize>::stack(InputIterator begin, InputIterator end,
				unsigned spacing)
			: _sizes(begin, end), _spacing(spacing)
		{	}


		template <typename InputIterator>
		inline hstack::hstack(InputIterator begin, InputIterator end, unsigned spacing)
			: base(begin, end, spacing)
		{	}


		template <typename InputIterator>
		inline vstack::vstack(InputIterator begin, InputIterator end, unsigned spacing)
			: base(begin, end, spacing)
		{	}
	}
}
