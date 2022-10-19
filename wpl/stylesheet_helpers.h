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

#include "concepts.h"
#include "stylesheet.h"

namespace wpl
{
	template <typename T>
	class styled_control : noncopyable
	{
	public:
		styled_control(std::shared_ptr<T> control_, const stylesheet &stylesheet_)
			: control(control_)
		{
			_changed_connection = stylesheet_.changed += [this, &stylesheet_] {
				this->control->apply_styles(stylesheet_);
			};
			control->apply_styles(stylesheet_);
		}

	public:
		const std::shared_ptr<T> control;

	private:
		slot_connection _changed_connection;
	};

	template <typename T>
	inline std::shared_ptr<T> apply_stylesheet(std::shared_ptr<T> control_, const stylesheet &stylesheet_)
	{	return std::shared_ptr<T>(std::make_shared< styled_control<T> >(control_, stylesheet_), control_.get());	}
}
