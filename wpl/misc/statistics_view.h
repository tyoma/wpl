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

#include "../visual.h"

#include <agge.text/richtext.h>
#include <algorithm>
#include <string.h>
#include <utility>
#include <vector>

namespace wpl
{
	namespace misc
	{
		class statistics_view : noncopyable
		{
		public:
			statistics_view(std::shared_ptr<gcontext::text_engine_type> text_services);

			template <typename T>
			void set_value(const char *parameter, const T &value, const char *unit);

			agge::rect_i update();

			void draw(gcontext &context, gcontext::rasterizer_ptr &rasterizer) const;

		private:
			struct cstring_less;
			struct variant
			{
				template <typename T>
				void set(const T &value, const char *unit_);

				enum type {	type_float, type_int, type_box	} current_type;
				union {	float f; int i; agge::box<int> b;	} value;
				std::string unit;
			};

		private:
			const std::shared_ptr<gcontext::text_engine_type> _text_services;
			agge::richtext_t _text;
			std::vector< std::pair<std::string, variant> > _values;
			agge::rect_i _last_invalid;
		};



		struct statistics_view::cstring_less
		{
			template <typename T>
			bool operator ()(const std::pair<std::string, T> &lhs, const std::pair<std::string, T> &rhs) const
			{	return lhs.first < rhs.first;	}

			template <typename T>
			bool operator ()(const char *lhs, const std::pair<std::string, T> &rhs) const
			{	return lhs < rhs.first;	}

			template <typename T>
			bool operator ()(const std::pair<std::string, T> &lhs, const char *rhs) const
			{	return lhs.first < rhs;	}
		};


		template <>
		inline void statistics_view::variant::set(const float &value_, const char *unit_)
		{	current_type = type_float, value.f = value_, unit = unit_;	}

		template <>
		inline void statistics_view::variant::set(const int &value_, const char *unit_)
		{	current_type = type_int, value.i = value_, unit = unit_;	}

		template <>
		inline void statistics_view::variant::set(const agge::box<int> &value_, const char *unit_)
		{	current_type = type_box, value.b = value_, unit = unit_;	}


		template <typename T>
		inline void statistics_view::set_value(const char *parameter, const T &value, const char *unit)
		{
			const auto r = std::equal_range(_values.begin(), _values.end(), parameter, cstring_less());
			auto p = r.first;

			if (r.first == r.second)
				p = _values.insert(r.first, std::make_pair(parameter, variant()));
			p->second.set(value, unit);
		}
	}
}
