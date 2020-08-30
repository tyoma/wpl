#include <wpl/factory.h>

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
		namespace
		{
			typedef pair< pair<shared_ptr<gcontext::renderer_type>, shared_ptr<gcontext::surface_type> >,
				shared_ptr<stylesheet> > form_init;
		}

		namespace mocks
		{
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

			vector<form_init> flog;

			factory::form_constructor create_fctor()
			{
				return [this] (shared_ptr<gcontext::renderer_type> renderer,
					shared_ptr<gcontext::surface_type> backbuffer, shared_ptr<stylesheet> stylesheet_) {

						assert_not_null(renderer);
						assert_not_null(backbuffer);

						flog.emplace_back(make_pair(renderer, backbuffer), stylesheet_);
						return shared_ptr<form>(new mocks::form);
				};
			}


			test( AttemptToCreateNotRegisteredFormFails )
			{
				// INIT
				shared_ptr<stylesheet> ss(new mocks::stylesheet);
				factory f(ss);

				// INIT / ACT / ASSERT
				assert_throws(f.create_form(), invalid_argument);
			}


			test( AttemptToCreateNotRegisteredControlFails )
			{
				// INIT
				shared_ptr<stylesheet> ss(new mocks::stylesheet);
				factory f(ss);

				// INIT / ACT / ASSERT
				assert_throws(f.create_control("button"), invalid_argument);
				assert_throws(f.create_control("listview"), invalid_argument);
			}


			test( FormIsCreatedWithThePredicatesSpecified )
			{
				// INIT
				shared_ptr<stylesheet> ss1(new mocks::stylesheet);
				factory f1(ss1);
				shared_ptr<stylesheet> ss2(new mocks::stylesheet);
				factory f2(ss2);

				f1.register_form(create_fctor());
				f2.register_form(create_fctor());

				// ACT
				f1.create_form();
				f2.create_form();

				// ASSERT
				assert_equal(2u, flog.size());
				assert_equal(ss1, flog[0].second);
				assert_equal(ss2, flog[1].second);

				// ACT
				f1.create_form();

				// ASSERT
				assert_equal(3u, flog.size());
				assert_equal(ss1, flog[0].second);
				assert_equal(ss2, flog[1].second);
			}


			test( SameRendererAndBackbufferAreSuppliedToFormConstructor )
			{
				// INIT
				shared_ptr<stylesheet> ss1(new mocks::stylesheet);
				factory f1(ss1);
				shared_ptr<stylesheet> ss2(new mocks::stylesheet);
				factory f2(ss2);

				f1.register_form(create_fctor());
				f2.register_form(create_fctor());

				// ACT
				f1.create_form();
				f1.create_form();
				f2.create_form();
				f2.create_form();

				// ASSERT
				assert_equal(4u, flog.size());

				assert_equal(flog[0].first.first, flog[1].first.first);
				assert_equal(flog[0].first.second, flog[1].first.second);

				assert_not_equal(flog[0].first.first, flog[2].first.first);
				assert_not_equal(flog[0].first.second, flog[2].first.second);

				assert_equal(flog[2].first.first, flog[3].first.first);
				assert_equal(flog[2].first.second, flog[3].first.second);
			}


			test( ControlsAreCreatedAccordinglyToMapping )
			{
				// INIT
				vector< pair<string, pair<const factory *, shared_ptr<stylesheet> > > > log;
				shared_ptr<stylesheet> ss(new mocks::stylesheet);
				factory f(ss);

				f.register_control("button", [&] (const factory &ff, shared_ptr<stylesheet> ss) {
					return log.emplace_back("button", make_pair(&ff, ss)), shared_ptr<control>();
				});
				f.register_control("listview", [&] (const factory &ff, shared_ptr<stylesheet> ss) {
					return log.emplace_back("listview", make_pair(&ff, ss)), shared_ptr<control>();
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
				shared_ptr<stylesheet> ss(new mocks::stylesheet);
				factory f(ss);

				f.register_form([&] (shared_ptr<gcontext::renderer_type>, shared_ptr<gcontext::surface_type>,
					shared_ptr<stylesheet>) {
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
				struct control : wpl::control
				{
					virtual std::shared_ptr<view> get_view() {	return nullptr;	}
				};

				const shared_ptr<control> c1(new control);
				const shared_ptr<control> c2(new control);
				shared_ptr<control> c = c1;
				shared_ptr<stylesheet> ss(new mocks::stylesheet);
				factory f(ss);

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
