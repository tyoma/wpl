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

#include "types.h"
#include "view_host.h"
#include "visual.h"

#include <string>

namespace wpl
{
	struct form : view_host
	{
		enum features {
			resizeable = 1 << 0,
			minimizable = 1 << 1,
			maximizable = 1 << 2,
		};

		virtual rect_i get_location() const = 0;
		virtual void set_location(const rect_i &location) = 0;
		virtual void center_parent() = 0;
		virtual void set_visible(bool value) = 0;
		virtual void set_caption(const std::string &caption) = 0;
		virtual void set_caption_icon(const gcontext::surface_type &icon) = 0;
		virtual void set_task_icon(const gcontext::surface_type &icon) = 0;
		virtual std::shared_ptr<form> create_child() = 0;
		virtual void set_features(unsigned /*features*/ features_) = 0;

		signal<void()> close;
	};
}
