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

#include "signals.h"

#include <agge/color.h>
#include <agge.text/font.h>

namespace wpl
{
	struct stylesheet
	{
		virtual agge::color get_color(const char *id) const = 0;
		virtual agge::font::ptr get_font(const char *id) const = 0;
		virtual agge::real_t get_value(const char *id) const = 0;

		signal<void ()> changed;
	};


	class stylesheet_db : public stylesheet
	{
	public:
		void set_color(const char *id, agge::color value);
		void set_font(const char *id, agge::font::ptr value);
		void set_value(const char *id, agge::real_t value);

	private:
		typedef std::unordered_map<std::string, agge::color> colors_t;
		typedef std::unordered_map<std::string, agge::font::ptr> fonts_t;
		typedef std::unordered_map<std::string, agge::real_t> values_t;

	private:
		virtual agge::color get_color(const char *id) const;
		virtual agge::font::ptr get_font(const char *id) const;
		virtual agge::real_t get_value(const char *id) const;

		template <typename ContainerT>
		typename ContainerT::mapped_type get_value(const ContainerT &container_, const char *id) const;

	private:
		colors_t _colors;
		fonts_t _fonts;
		values_t _values;
	};
}
