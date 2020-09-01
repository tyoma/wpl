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

#include <wpl/factory.h>

#include <wpl/controls/scroller.h>
#include <wpl/win32/combobox.h>
#include <wpl/win32/controls.h>
#include <wpl/win32/form.h>
#include <wpl/win32/listview.h>

using namespace std;

namespace wpl
{
	shared_ptr<factory> factory::create_default(const shared_ptr<stylesheet> &stylesheet_)
	{
		shared_ptr<factory> f(new factory(stylesheet_));

		f->register_form([] (shared_ptr<gcontext::renderer_type>, shared_ptr<gcontext::surface_type>, shared_ptr<stylesheet>){
			return shared_ptr<form>(new win32::form);
		});

		f->register_control("button", [] (const factory &, shared_ptr<stylesheet>) {
			return shared_ptr<button>(new win32::button);
		});
		f->register_control("link", [] (const factory &, shared_ptr<stylesheet>) {
			return shared_ptr<link>(new win32::link);
		});
		f->register_control("combobox", [] (const factory &, shared_ptr<stylesheet>) {
			return shared_ptr<combobox>(new win32::combobox);
		});
		f->register_control("listview", [] (const factory &, shared_ptr<stylesheet>) {
			return shared_ptr<listview>(new win32::listview);
		});
		f->register_control("hscroller", [] (const factory &, shared_ptr<stylesheet>) {
			return shared_ptr<scroller>(new controls::scroller(controls::scroller::horizontal));
		});
		f->register_control("vscroller", [] (const factory &, shared_ptr<stylesheet>) {
			return shared_ptr<scroller>(new controls::scroller(controls::scroller::vertical));
		});

		return f;
	}
}
