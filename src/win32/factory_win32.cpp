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
#include <wpl/stylesheet_helpers.h>
#include <wpl/win32/cursor_manager.h>
#include <wpl/win32/font_loader.h>
#include <wpl/win32/font_manager.h>
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

		template <typename T>
		struct stylesheet_applier
		{
			stylesheet_applier(shared_ptr<T> control_, const stylesheet &stylesheet_,
				shared_ptr<win32::font_manager> font_manager)
				: control(control_)
			{
				const auto apply = [this, &stylesheet_, font_manager] {
					this->control->apply_styles(stylesheet_, *font_manager);
				};

				style_changed_conn = stylesheet_.changed += apply;
				apply();
			}

			shared_ptr<T> control;
			slot_connection style_changed_conn;
		};

		template <typename T>
		shared_ptr<T> apply_stylesheet(shared_ptr<T> control, const stylesheet &stylesheet_,
			shared_ptr<win32::font_manager> font_manager)
		{
			const auto wrapped = make_shared< stylesheet_applier<T> >(control, stylesheet_, font_manager);
			return shared_ptr<T>(wrapped, wrapped->control.get());
		}
	}

	shared_ptr<factory> factory::create_default(const shared_ptr<stylesheet> &stylesheet_)
	{
		shared_ptr<text_engine_composite> tec(new text_engine_composite);
		factory_context context = {
			shared_ptr<gcontext::surface_type>(new gcontext::surface_type(1, 1, 16)),
			shared_ptr<gcontext::renderer_type>(new gcontext::renderer_type(2)),
			shared_ptr<gcontext::text_engine_type>(tec, &tec->text_engine),
			stylesheet_,
			make_shared<win32::cursor_manager>(),
			win32::clock,
			win32::queue(),
		};

		return create_default(context);
	}

	void factory::setup_default(factory &factory_)
	{
		const auto font_manager = make_shared<win32::font_manager>();

		factory_.register_form([] (const form_context &context) {
			return shared_ptr<form>(new win32::form(context));
		});

		factory_.register_control("background", [] (const factory &, const control_context &context) {
			return apply_stylesheet(make_shared<controls::solid_background>(), *context.stylesheet_);
		});
		factory_.register_control("button", [font_manager] (const factory &, const control_context &context) {
			return apply_stylesheet(make_shared<win32::button>(), *context.stylesheet_, font_manager);
		});
		factory_.register_control("link", [font_manager] (const factory &, const control_context &context) {
			return apply_stylesheet(make_shared<win32::link>(), *context.stylesheet_, font_manager);
		});
		factory_.register_control("combobox", [font_manager] (const factory &, const control_context &context) {
			return apply_stylesheet(make_shared<win32::combobox>(), *context.stylesheet_, font_manager);
		});
		factory_.register_control("header", [] (const factory &, const control_context &context) {
			return apply_stylesheet(make_shared<controls::header_basic>(context.cursor_manager_), *context.stylesheet_);
		});
		factory_.register_control("listview", [] (const factory &f, const control_context &context) -> shared_ptr<control> {
			return apply_stylesheet(make_shared< controls::listview_composite<controls::listview_basic> >(f, context), *context.stylesheet_);
		});
		factory_.register_control("listview.native", [font_manager] (const factory &, const control_context &context) {
			return apply_stylesheet(make_shared<win32::listview>(), *context.stylesheet_, font_manager);
		});
		factory_.register_control("hscroller", [] (const factory &, const control_context &) {
			return shared_ptr<control>(new controls::scroller(controls::scroller::horizontal));
		});
		factory_.register_control("vscroller", [] (const factory &, const control_context &) {
			return shared_ptr<control>(new controls::scroller(controls::scroller::vertical));
		});
	}
}
