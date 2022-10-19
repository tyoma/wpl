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

#include "control.h"
#include "models.h"

#include <agge.text/richtext.h>
#include <agge.text/types.h>
#include <string>

namespace wpl
{
	struct text_container
	{
		virtual void set_text(const agge::richtext_modifier_t &text) = 0;
		virtual void set_halign(agge::text_alignment value) = 0;
		virtual void set_valign(agge::text_alignment value) = 0;
	};

	template <typename ValueT>
	struct value_editor
	{
		typedef ValueT value_type;

		virtual bool get_value(value_type &value) const = 0;
		virtual void set_value(const value_type &value) = 0;

		signal<void ()> changed;
	};

	template <typename ModelT>
	struct unimodel_control : control
	{
		typedef ModelT model_type;

		virtual void set_model(std::shared_ptr<model_type> model) = 0;
	};

	struct label : control, text_container
	{
	};

	struct button : control, text_container
	{
		signal<void ()> clicked;
	};

	struct link : control, text_container
	{
		signal<void (size_t item, const std::string &link_text)> clicked;
	};

	struct editbox : control, value_editor<std::string>
	{
		signal<void (value_type &value)> accept;
		signal<void (wchar_t &character)> translate_char;
	};

	struct combobox : control
	{
		typedef list_model<std::string> model_t;

		virtual void set_model(std::shared_ptr<model_t> model) = 0;
		virtual void select(model_t::index_type item) = 0;

		signal<void (model_t::index_type item)> selection_changed;
	};

	struct listview : control
	{
		virtual void set_columns_model(std::shared_ptr<headers_model> model) = 0;
		virtual void set_selection_model(std::shared_ptr<dynamic_set_model> model) = 0;
		virtual void set_model(std::shared_ptr<richtext_table_model> model) = 0;

		virtual void focus(table_model_base::index_type item) = 0;

		signal<void (table_model_base::index_type /*item*/)> item_activate;
	};

	typedef unimodel_control<headers_model> header;
	typedef unimodel_control<sliding_window_model> range_slider;
	typedef unimodel_control<scroll_model> scroller;
}
