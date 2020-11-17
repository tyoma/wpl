//	Copyright (c) 2011-2020 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "../cursor.h"

#include <map>
#include <unordered_map>
#include <vector>

namespace wpl
{
	namespace win32
	{
		class cursor_manager : public wpl::cursor_manager
		{
		public:
			cursor_manager();
			~cursor_manager();

			virtual std::shared_ptr<const cursor> get(standard_cursor id) const;
			virtual void set(std::shared_ptr<const cursor> cursor_);
			virtual void push(std::shared_ptr<const cursor> cursor_);
			virtual void pop();

		private:
			typedef std::map<const cursor *, std::shared_ptr<void> /*HCURSOR*/> cached_cursors;
			typedef std::shared_ptr<cached_cursors> cached_cursors_ptr;

		private:
			std::shared_ptr<const cursor> associate(std::shared_ptr<void> hcursor) const;
			static std::shared_ptr<void> get_cursor(cursor_manager::standard_cursor id);

		private:
			mutable std::unordered_map< standard_cursor, std::shared_ptr<const cursor> > _standard_cursors;
			cached_cursors_ptr _cursors;
			std::vector<void *> _cursor_stack;
		};
	}
}
