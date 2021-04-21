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

#include "signal.h"

#include <agge.text/richtext.h>
#include <memory>
#include <string>

namespace wpl
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


	struct scroll_model
	{
		virtual std::pair<double /*range_min*/, double /*range_width*/> get_range() const = 0;
		virtual std::pair<double /*window_min*/, double /*window_width*/> get_window() const = 0;
		virtual double get_increment() const = 0;
		virtual void scrolling(bool begins) = 0;
		virtual void scroll_window(double window_min, double window_width) = 0;

		signal<void (bool invalidate_range)> invalidate;
	};


	template <typename T>
	struct list_model : index_traits
	{
		typedef T value_type;

		virtual index_type get_count() const throw() = 0;
		virtual void get_value(index_type index, value_type &value) const = 0;
		virtual std::shared_ptr<const trackable> track(index_type item) const;

		signal<void (index_type item)> invalidate;
	};


	struct columns_model : list_model<short int>
	{
		virtual void set_width(index_type index, short int width);
	};


	struct headers_model : columns_model
	{
		virtual std::pair<index_type, bool> get_sort_order() const throw();
		virtual void get_caption(index_type index, agge::richtext_t &caption) const = 0;
		virtual void activate_column(index_type column);

		signal<void (index_type /*new_ordering_column*/, bool /*ascending*/)> sort_order_changed;
	};


	struct hierarchical_headers_model : headers_model
	{
		virtual index_type get_levels_count() const throw() = 0;
		virtual std::shared_ptr<headers_model> get_level(index_type higher_level) = 0;
	};


	struct table_model_base : index_traits
	{
		virtual index_type get_count() const throw() = 0;
		virtual void precache(index_type from, index_type count);
		virtual std::shared_ptr<const trackable> track(index_type row) const;

		signal<void (index_type row)> invalidate; // It is model's responsibility to invalidate itself on count changes.
	};


	template <typename T>
	struct table_model : table_model_base
	{
		typedef T value_type;

		virtual void get_text(index_type row, index_type column, value_type &value) const = 0;
	};

	typedef table_model<std::string> string_table_model;
	typedef table_model<agge::richtext_t> richtext_table_model;



	inline index_traits::index_type index_traits::npos()
	{	return static_cast<index_type>(-1);	}


	template <typename T>
	inline std::shared_ptr<const trackable> list_model<T>::track(index_type /*row*/) const
	{	return std::shared_ptr<const trackable>();	}


	inline void columns_model::set_width(index_type /*index*/, short int /*width*/)
	{	}


	inline std::pair<headers_model::index_type, bool> headers_model::get_sort_order() const throw()
	{	return std::make_pair(npos(), false);	}

	inline void headers_model::activate_column(index_type /*column*/)
	{	}


	inline void table_model_base::precache(index_type /*from*/, index_type /*count*/)
	{	}

	inline std::shared_ptr<const trackable> table_model_base::track(index_type /*row*/) const
	{	return std::shared_ptr<trackable>();	}
}
