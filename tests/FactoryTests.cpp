#include <wpl/factory.h>

#include <tests/common/mock-control.h>
#include <tests/common/Mockups.h>

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
			class form : public wpl::form
			{
			private:
				virtual void set_root(shared_ptr<wpl::control> /*root*/) override { throw 0; }

				virtual rect_i get_location() const override { throw 0; }
				virtual void set_location(const rect_i &/*location*/) override { throw 0; };
				virtual void set_visible(bool /*value*/) override { throw 0; };
				virtual void set_caption(const wstring &/*caption*/) override { throw 0; };
				virtual void set_caption_icon(const gcontext::surface_type &/*icon*/) override { throw 0; };
				virtual void set_task_icon(const gcontext::surface_type &/*icon*/) override { throw 0; };
				virtual shared_ptr<wpl::form> create_child() override { throw 0; };
				virtual void set_features(unsigned /*features*/ /*features_*/) override { throw 0; };
			};

			class stylesheet : public wpl::stylesheet
			{
				virtual agge::color get_color(const char * /*id*/) const override { throw 0; }
				virtual agge::font::ptr get_font(const char * /*id*/) const override { throw 0; }
				virtual agge::real_t get_value(const char * /*id*/) const override { throw 0; }
			};
		}

		begin_test_suite( FactoryTests )
			mocks::font_loader fake_loader;
			factory_context context;

			init( Init )
			{
				context.backbuffer.reset(new gcontext::surface_type(1, 1, 16));
				context.renderer.reset(new gcontext::renderer_type(1));
				context.text_engine.reset(new gcontext::text_engine_type(fake_loader, 0));
				context.stylesheet_.reset(new mocks::stylesheet);
				context.cursor_manager_.reset(new mocks::cursor_manager);
			}


			test( AttemptToCreateNotRegisteredFormFails )
			{
				// INIT
				factory f(context);

				// INIT / ACT / ASSERT
				assert_throws(f.create_form(), invalid_argument);
			}


			test( AttemptToCreateNotRegisteredControlFails )
			{
				// INIT
				factory f(context);

				// INIT / ACT / ASSERT
				assert_throws(f.create_control("button"), invalid_argument);
				assert_throws(f.create_control("listview"), invalid_argument);
			}


			test( FormIsCreatedWithThePredicatesSpecified )
			{
				// INIT
				factory f1(context);
				factory_context context2 = {
					shared_ptr<gcontext::surface_type>(new gcontext::surface_type(1, 1, 16)),
					shared_ptr<gcontext::renderer_type>(new gcontext::renderer_type(1)),
					shared_ptr<gcontext::text_engine_type>(new gcontext::text_engine_type(fake_loader, 0)),
					shared_ptr<stylesheet>(new mocks::stylesheet),
				};
				factory f2(context2);
				auto calls_1 = 0;
				auto calls_2 = 0;

				// ACT / ASSERT
				f1.register_form([&] (const form_context &context) -> shared_ptr<form> {

					assert_equal(this->context.backbuffer, context.backbuffer);
					assert_equal(this->context.renderer, context.renderer);
					assert_equal(this->context.text_engine, context.text_engine);
					assert_equal(this->context.stylesheet_, context.stylesheet_);
					calls_1++;
					return shared_ptr<form>(new mocks::form);
				});
				f2.register_form([&] (const form_context &context) -> shared_ptr<form> {

					assert_equal(context2.backbuffer, context.backbuffer);
					assert_equal(context2.renderer, context.renderer);
					assert_equal(context2.text_engine, context.text_engine);
					assert_equal(context2.stylesheet_, context.stylesheet_);
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
				vector<string> log;
				factory f(context);

				f.register_control("button", [&] (const factory &, const control_context &) {
					return log.push_back("button"), shared_ptr<control>();
				});
				f.register_control("listview", [&] (const factory &, const control_context &) {
					return log.push_back("listview"), shared_ptr<control>();
				});

				// ACT
				f.create_control("button");
				f.create_control("listview");

				// ASSERT
				assert_equal(2u, log.size());
				assert_equal("button", log[0]);
				assert_equal("listview", log[1]);
			}


			test( ControlContextIsFormedFromFactoryContextOnCreation )
			{
				// INIT
				pair<const factory *, control_context> passed_context;
				timestamp ts;
				vector< pair<queue_task, timespan> > log;
				bool schedule_result = false;

				context.clock_ = [&] {	return ts;	};
				context.queue_ = [&] (const queue_task &t, timespan d) -> bool {
					log.push_back(make_pair(t, d));
					return schedule_result;
				};

				factory f(context);

				f.register_control("button", [&] (const factory &ff, const control_context &cc) {
					return passed_context = make_pair(&ff, cc), shared_ptr<control>();
				});

				// ACT
				f.create_control("button");

				// ASSERT
				int callid = 0;

				assert_equal(&f, passed_context.first);
				assert_equal(context.stylesheet_, passed_context.second.stylesheet_);
				assert_equal(context.cursor_manager_, passed_context.second.cursor_manager_);
				assert_equal(context.text_engine, passed_context.second.text_services);
				ts = 12345678;
				assert_equal(ts, passed_context.second.clock_());
				ts = 912345678;
				assert_equal(ts, passed_context.second.clock_());
				assert_is_false(passed_context.second.queue_([&] { callid = 17; }, 10));
				schedule_result = true;
				assert_is_true(passed_context.second.queue_([&] { callid = 179; }, 0));
				assert_equal(2u, log.size());
				log[0].first();
				assert_equal(17, callid);
				assert_equal(10, log[0].second);
				log[1].first();
				assert_equal(179, callid);
				assert_equal(0, log[1].second);
			}


			test( ExpectedObjectsAreReturnedFromCreateForm )
			{
				// INIT
				const shared_ptr<mocks::form> frm1(new mocks::form);
				const shared_ptr<mocks::form> frm2(new mocks::form);
				shared_ptr<mocks::form> frm = frm1;
				factory f(context);

				f.register_form([&] (const form_context &) {
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
				factory f(context);

				f.register_control("button", [&] (const factory &/*ff*/, const control_context &/*cc*/) { return c; });

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
