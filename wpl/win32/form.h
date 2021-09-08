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

#include "../factory_context.h"
#include "../form.h"
#include "helpers.h"
#include "utf8.h"
#include "window.h"

#include <memory>

namespace wpl
{
	namespace win32
	{
		class view_host;

		class form : public wpl::form
		{
		public:
			form(const form_context &context, HWND howner = NULL);
			~form();

		private:
			// view_host methods
			virtual void set_root(std::shared_ptr<control> root) override;

			// form methods
			virtual rect_i get_location() const override;
			virtual void set_location(const rect_i &location) override;
			virtual void center_parent() override;
			virtual void set_visible(bool value) override;
			virtual void set_caption(const std::string &caption) override;
			virtual void set_caption_icon(const gcontext::surface_type &icon) override;
			virtual void set_task_icon(const gcontext::surface_type &icon) override;
			virtual std::shared_ptr<wpl::form> create_child() override;
			virtual void set_features(unsigned /*features*/ features_) override;

			LRESULT wndproc(UINT message, WPARAM wparam, LPARAM lparam, const window::original_handler_t &previous);

		private:
			helpers::window_handle _hwnd;
			std::shared_ptr<win32::view_host> _host;
			utf_converter _converter;
		};
	}
}
