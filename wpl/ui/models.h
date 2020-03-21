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

#include <wpl/base/concepts.h>
#include <wpl/base/signals.h>

#include <memory>
#include <string>

namespace wpl
{
	namespace ui
	{
		struct index_traits
		{
			typedef size_t index_type;

			static index_type npos();
		};

		struct trackable : index_traits
		{
			virtual index_type index() const = 0;
		};

		struct list_model : index_traits
		{
			virtual index_type get_count() const throw() = 0;
			virtual void get_text(index_type index, std::wstring &text) const = 0;
			// virtual std::shared_ptr<const trackable> track(index_type row) const;

			signal<void (index_type new_count)> invalidated;
		};

		struct table_model : index_traits
		{
			virtual index_type get_count() const throw() = 0;
			virtual void get_text(index_type row, index_type column, std::wstring &text) const = 0;
			virtual void set_order(index_type column, bool ascending) = 0;
			virtual void precache(index_type from, index_type count) const;
			virtual std::shared_ptr<const trackable> track(index_type row) const;

			signal<void (index_type new_count)> invalidated;
		};



		inline index_traits::index_type index_traits::npos()
		{	return static_cast<index_type>(-1);	}


		inline void table_model::precache(index_type /*from*/, index_type /*count*/) const
		{	}

		inline std::shared_ptr<const trackable> table_model::track(index_type /*row*/) const
		{	return std::shared_ptr<trackable>();	}
	}
}
