#include <wpl/factory.h>

#include "Mockups.h"

#include <wpl/control.h>
#include <wpl/form.h>
#include <wpl/stylesheet.h>

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace mocks
		{
			struct control : wpl::control
			{
				virtual std::shared_ptr<view> get_view() {	return std::shared_ptr<view>();	}
			};

			class form : public wpl::form
			{
			private:
				virtual void set_view(const shared_ptr<view> &/*v*/) { throw 0; }
				virtual void set_background_color(agge::color /*color_*/) { throw 0; }

				virtual view_location get_location() const { throw 0; }
				virtual void set_location(const view_location &/*location*/) { throw 0; };
				virtual void set_visible(bool /*value*/) { throw 0; };
				virtual void set_caption(const wstring &/*caption*/) { throw 0; };
				virtual void set_caption_icon(const gcontext::surface_type &/*icon*/) { throw 0; };
				virtual void set_task_icon(const gcontext::surface_type &/*icon*/) { throw 0; };
				virtual shared_ptr<wpl::form> create_child() { throw 0; };
				virtual void set_style(unsigned /*styles*/ /*style*/) { throw 0; };
				virtual void set_font(const font &/*font_*/){ throw 0; };
			};

			class stylesheet : public wpl::stylesheet
			{
				virtual agge::color get_color(const char * /*id*/) const { throw 0; }
				virtual shared_ptr<agge::font> get_font(const char * /*id*/) const { throw 0; }
				virtual agge::real_t get_value(const char * /*id*/) const { throw 0; }
			};
		}

		begin_test_suite( FactoryTests )
			mocks::font_loader fake_loader;
			shared_ptr<gcontext::text_engine_type> text_engine;
			shared_ptr<gcontext::surface_type> backbuffer;
			shared_ptr<gcontext::renderer_type> renderer;
			shared_ptr<stylesheet> ss;

			init( Init )
			{
				backbuffer.reset(new gcontext::surface_type(1, 1, 16));
				renderer.reset(new gcontext::renderer_type(1));
				text_engine.reset(new gcontext::text_engine_type(fake_loader, 0));
				ss.reset(new mocks::stylesheet);
			}


			test( AttemptToCreateNotRegisteredFormFails )
			{
				// INIT
				factory f(backbuffer, renderer, text_engine, ss);

				// INIT / ACT / ASSERT
				assert_throws(f.create_form(), invalid_argument);
			}


			test( AttemptToCreateNotRegisteredControlFails )
			{
				// INIT
				factory f(backbuffer, renderer, text_engine, ss);

				// INIT / ACT / ASSERT
				assert_throws(f.create_control("button"), invalid_argument);
				assert_throws(f.create_control("listview"), invalid_argument);
			}


			test( FormIsCreatedWithThePredicatesSpecified )
			{
				// INIT
				factory f1(backbuffer, renderer, text_engine, ss);
				shared_ptr<gcontext::surface_type> b2(new gcontext::surface_type(1, 1, 16));
				shared_ptr<gcontext::renderer_type> r2(new gcontext::renderer_type(1));
				shared_ptr<gcontext::text_engine_type> te2(new gcontext::text_engine_type(fake_loader, 0));
				shared_ptr<stylesheet> ss2(new mocks::stylesheet);
				factory f2(b2, r2, te2, ss2);
				auto calls_1 = 0;
				auto calls_2 = 0;

				// ACT / ASSERT
				f1.register_form([&] (shared_ptr<gcontext::surface_type> backbuffer, shared_ptr<gcontext::renderer_type> renderer,
					shared_ptr<gcontext::text_engine_type> text_engine, shared_ptr<stylesheet> stylesheet_) -> shared_ptr<form> {

					assert_equal(this->backbuffer, backbuffer);
					assert_equal(this->renderer, renderer);
					assert_equal(this->text_engine, text_engine);
					assert_equal(this->ss, stylesheet_);
					calls_1++;
					return shared_ptr<form>(new mocks::form);
				});
				f2.register_form([&] (shared_ptr<gcontext::surface_type> backbuffer, shared_ptr<gcontext::renderer_type> renderer,
					shared_ptr<gcontext::text_engine_type> text_engine, shared_ptr<stylesheet> stylesheet_) -> shared_ptr<form> {

					assert_equal(b2, backbuffer);
					assert_equal(r2, renderer);
					assert_equal(te2, text_engine);
					assert_equal(ss2, stylesheet_);
					calls_2++;
					return shared_ptr<form>(new mocks::form);
				});

				// ACT
				f1.create_form();
				f2.create_form();

				// ASSERT
				assert_equal(1, calls_1);
				assert_equal(1, calls_2);
			}


			test( ControlsAreCreatedAccordinglyToMapping )
			{
				// INIT
				vector< pair<string, pair<const factory *, shared_ptr<stylesheet> > > > log;
				factory f(backbuffer, renderer, text_engine, ss);

				f.register_control("button", [&] (const factory &ff, shared_ptr<stylesheet> ss) {
					return log.push_back(make_pair("button", make_pair(&ff, ss))), shared_ptr<control>();
				});
				f.register_control("listview", [&] (const factory &ff, shared_ptr<stylesheet> ss) {
					return log.push_back(make_pair("listview", make_pair(&ff, ss))), shared_ptr<control>();
				});

				// ACT
				f.create_control("button");
				f.create_control("listview");

				// ASSERT
				assert_equal(2u, log.size());
				assert_equal("button", log[0].first);
				assert_equal(&f, log[0].second.first);
				assert_equal(ss, log[0].second.second);
				assert_equal("listview", log[1].first);
				assert_equal(&f, log[1].second.first);
				assert_equal(ss, log[1].second.second);
			}


			test( ExpectedObjectsAreReturnedFromCreateForm )
			{
				// INIT
				const shared_ptr<mocks::form> frm1(new mocks::form);
				const shared_ptr<mocks::form> frm2(new mocks::form);
				shared_ptr<mocks::form> frm = frm1;
				factory f(backbuffer, renderer, text_engine, ss);

				f.register_form([&] (shared_ptr<void>, shared_ptr<void>, shared_ptr<void>, shared_ptr<void>) {
					return frm;
				});

				// ACT / ASSERT
				assert_equal(frm1, f.create_form());

				// INIT
				frm = frm2;

				// ACT / ASSERT
				assert_equal(frm2, f.create_form());
			}


			test( ExpectedObjectsAreReturnedFromCreateControl )
			{
				const shared_ptr<control> c1(new mocks::control);
				const shared_ptr<control> c2(new mocks::control);
				shared_ptr<control> c = c1;
				factory f(backbuffer, renderer, text_engine, ss);

				f.register_control("button", [&] (const factory &/*ff*/, shared_ptr<stylesheet> /*ss*/) { return c; });

				// ACT / ASSERT
				assert_equal(c1, f.create_control("button"));

				// INIT
				c = c2;

				// ACT / ASSERT
				assert_equal(c2, f.create_control("button"));
			}
		end_test_suite
	}
}
