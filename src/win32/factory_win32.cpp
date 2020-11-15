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

#include <wpl/controls/header_basic.h>
#include <wpl/controls/listview_composite.h>
#include <wpl/controls/listview_basic.h>
#include <wpl/controls/scroller.h>
#include <wpl/win32/font_loader.h>
#include <wpl/win32/combobox.h>
#include <wpl/win32/controls.h>
#include <wpl/win32/form.h>
#include <wpl/win32/listview.h>
#include <wpl/win32/queue.h>

using namespace std;

namespace wpl
{
	namespace
	{
		struct text_engine_composite : noncopyable
		{
			text_engine_composite()
				: text_engine(loader, 4)
			{	}

			win32::font_loader loader;
			gcontext::text_engine_type text_engine;
		};
	}

	shared_ptr<factory> factory::create_default(const shared_ptr<stylesheet> &stylesheet_)
	{
		shared_ptr<text_engine_composite> tec(new text_engine_composite);
		factory_context context = {
			shared_ptr<gcontext::surface_type>(new gcontext::surface_type(1, 1, 16)),
			shared_ptr<gcontext::renderer_type>(new gcontext::renderer_type(2)),
			shared_ptr<gcontext::text_engine_type>(tec, &tec->text_engine),
			stylesheet_,
			shared_ptr<cursor_manager>(),
			win32::clock,
			win32::queue(),
		};

		return create_default(context);
	}

	void factory::setup_default(factory &factory_)
	{
		factory_.register_form([] (const form_context &context) {
			return shared_ptr<form>(new win32::form(context));
		});

		factory_.register_control("button", [] (const factory &, const control_context &) {
			return shared_ptr<control>(new win32::button);
		});
		factory_.register_control("link", [] (const factory &, const control_context &) {
			return shared_ptr<control>(new win32::link);
		});
		factory_.register_control("combobox", [] (const factory &, const control_context &) {
			return shared_ptr<control>(new win32::combobox);
		});
		factory_.register_control("header", [] (const factory &, const control_context &context) {
			return shared_ptr<control>(new controls::header_basic(context.stylesheet_));
		});
		factory_.register_control("listview", [] (const factory &f, const control_context &context) {
			return controls::create_listview<controls::listview_basic>(f, context);
		});
		factory_.register_control("listview.native", [] (const factory &, const control_context &) {
			return shared_ptr<control>(new win32::listview);
		});
		factory_.register_control("hscroller", [] (const factory &, const control_context &) {
			return shared_ptr<control>(new controls::scroller(controls::scroller::horizontal));
		});
		factory_.register_control("vscroller", [] (const factory &, const control_context &) {
			return shared_ptr<control>(new controls::scroller(controls::scroller::vertical));
		});
	}
}
