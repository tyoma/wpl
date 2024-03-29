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

#include "text_container.h"
#include "helpers.h"

#include "../controls.h"
#include <memory>

namespace wpl
{
	struct button;
	struct link;
	struct view_host;

	namespace win32
	{
		class button : public text_container_impl<wpl::button>
		{
		public:
			button();

		private:
			virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) override;

			virtual HWND materialize(HWND hparent) override;
			virtual LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam,
				const window::original_handler_t &handler) override;
		};


		class link : public text_container_impl<wpl::link>
		{
		public:
			link();

		private:
			virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) override;

			virtual HWND materialize(HWND hparent) override;
			virtual LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam,
				const window::original_handler_t &handler) override;

			virtual void set_halign(agge::text_alignment value) override;

		private:
			utf_converter _converter;
		};


		class editbox : public wpl::editbox, public native_view
		{
		public:
			editbox();

			virtual bool get_value(value_type &value) const override;
			virtual void set_value(const value_type &value) override;

		private:
			virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) override;

			virtual HWND materialize(HWND hparent) override;
			virtual LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam,
				const window::original_handler_t &handler) override;

		private:
			std::string _text;
			helpers::window_text _converter;
		};
	}
}
