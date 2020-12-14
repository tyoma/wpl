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

#include <wpl/controls/background.h>
#include <wpl/controls/header_basic.h>
#include <wpl/controls/listview_composite.h>
#include <wpl/controls/listview_basic.h>
#include <wpl/controls/scroller.h>
#include <wpl/macos/form.h>
#include <wpl/stylesheet_helpers.h>

using namespace std;

namespace wpl
{
	shared_ptr<factory> factory::create_default(const shared_ptr<stylesheet> &stylesheet_)
	{
		factory_context context = {
			shared_ptr<gcontext::surface_type>(new gcontext::surface_type(1, 1, 16)),
			shared_ptr<gcontext::renderer_type>(new gcontext::renderer_type(2)),
			nullptr,
			stylesheet_,
			nullptr,
			[] {	return timestamp();	},
			[] (const function<void ()> &, timespan) -> bool {	return false;	},
		};

		return create_default(context);
	}

	void factory::setup_default(factory &factory_)
	{
		factory_.register_form([] (const form_context &context_) {
			return make_shared<macos::form>(context_);
		});
		factory_.register_control("background", [] (const factory &, const control_context &context) {
			return apply_stylesheet(make_shared<controls::solid_background>(), *context.stylesheet_);
		});
		factory_.register_control("header", [] (const factory &, const control_context &context) {
			return apply_stylesheet(make_shared<controls::header_basic>(context.cursor_manager_), *context.stylesheet_);
		});
		factory_.register_control("listview", [] (const factory &f, const control_context &context) -> shared_ptr<control> {
			return apply_stylesheet(make_shared< controls::listview_composite<controls::listview_basic> >(f, context), *context.stylesheet_);
		});
		factory_.register_control("hscroller", [] (const factory &, const control_context &) {
			return shared_ptr<control>(new controls::scroller(controls::scroller::horizontal));
		});
		factory_.register_control("vscroller", [] (const factory &, const control_context &) {
			return shared_ptr<control>(new controls::scroller(controls::scroller::vertical));
		});
	}
}
