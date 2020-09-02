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

#include "concepts.h"
#include "stylesheet.h"
#include "visual.h"

#include <functional>
#include <memory>
#include <unordered_map>

namespace wpl
{
	struct control;
	struct form;
	struct stylesheet;

	class factory : noncopyable
	{
	public:
		typedef std::function<std::shared_ptr<form> (const std::shared_ptr<gcontext::surface_type> &backbuffer,
			const std::shared_ptr<gcontext::renderer_type> &renderer,
			const std::shared_ptr<gcontext::text_engine_type> &text_engine,
			const std::shared_ptr<stylesheet> &stylesheet_)> form_constructor;
		typedef std::function<std::shared_ptr<control> (const factory &factory_,
			const std::shared_ptr<stylesheet> &stylesheet_)> control_constructor;

	public:
		factory(const std::shared_ptr<gcontext::surface_type> &backbuffer,
			const std::shared_ptr<gcontext::renderer_type> &renderer,
			const std::shared_ptr<gcontext::text_engine_type> &text_engine, const std::shared_ptr<stylesheet> &stylesheet_);

		void register_form(const form_constructor &constructor);
		void register_control(const char *type, const control_constructor &constructor);

		std::shared_ptr<form> create_form() const;
		std::shared_ptr<control> create_control(const char *type) const;

		static std::shared_ptr<factory> create_default(const std::shared_ptr<stylesheet> &stylesheet_);
		static std::shared_ptr<factory> create_default(const std::shared_ptr<gcontext::surface_type> &backbuffer,
			const std::shared_ptr<gcontext::renderer_type> &renderer,
			const std::shared_ptr<gcontext::text_engine_type> &text_engine,
			const std::shared_ptr<stylesheet> &stylesheet_);
		static void setup_default(factory &factory_);

	private:
		const std::shared_ptr<gcontext::surface_type> _backbuffer;
		const std::shared_ptr<gcontext::renderer_type> _renderer;
		const std::shared_ptr<gcontext::text_engine_type> _text_engine;
		const std::shared_ptr<stylesheet> _stylesheet;
		form_constructor _default_form_constructor;
		std::unordered_map<std::string, control_constructor> _control_constructors;
	};
}
