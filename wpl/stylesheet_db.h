//	Copyright (c) 2011-2022 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "stylesheet.h"

namespace wpl
{
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
		virtual agge::color get_color(const char *id) const override;
		virtual agge::font::ptr get_font(const char *id) const override;
		virtual agge::real_t get_value(const char *id) const override;

		template <typename ContainerT>
		typename ContainerT::mapped_type get_value(const ContainerT &container_, const char *id) const;

	private:
		colors_t _colors;
		fonts_t _fonts;
		values_t _values;
	};
}
