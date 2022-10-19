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

#include "queue.h"
#include "visual.h"

namespace wpl
{
	struct cursor_manager;
	struct stylesheet;

	struct factory_context
	{
		std::shared_ptr<gcontext::surface_type> backbuffer;
		std::shared_ptr<gcontext::renderer_type> renderer;
		std::shared_ptr<gcontext::text_engine_type> text_engine;
		std::shared_ptr<stylesheet> stylesheet_;
		std::shared_ptr<cursor_manager> cursor_manager_;
		clock clock_;
		queue queue_;
	};

	typedef factory_context form_context;

	struct control_context
	{
		std::shared_ptr<gcontext::text_engine_type> text_services;
		std::shared_ptr<stylesheet> stylesheet_;
		std::shared_ptr<cursor_manager> cursor_manager_;
		clock clock_;
		queue queue_;
	};
}
