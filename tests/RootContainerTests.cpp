#include <wpl/root_container.h>

#include "Mockups.h"

#include <agge.text/text_engine.h>
#include <ut/assert.h>
#include <ut/test.h>
#include <wpl/cursor.h>
#include <wpl/stylesheet_db.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace mocks
		{
			class view : public wpl::view
			{
			public:
				function<void (gcontext &ctx)> on_draw;
				function<void ()> on_mouse_leave;

			private:
				virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &/*rasterizer*/) const
				{	if (on_draw) on_draw(ctx);	}

				virtual void mouse_leave()
				{	if (on_mouse_leave) on_mouse_leave();	}
			};
		}

		begin_test_suite( RootContainerTests )
			shared_ptr<mocks::cursor_manager> cursor_manager_;
			visual::positioned_native_views nviews;

			shared_ptr<gcontext::surface_type> surface;
			shared_ptr<gcontext::renderer_type> renderer;
			mocks::font_loader fake_loader;
			shared_ptr<gcontext::text_engine_type> text_engine;
			gcontext::rasterizer_ptr ras;
			shared_ptr<gcontext> ctx;


			init( Init )
			{
				cursor_manager_ = make_shared<mocks::cursor_manager>();
				surface.reset(new gcontext::surface_type(200, 200, 0));
				renderer.reset(new gcontext::renderer_type(1));
				text_engine.reset(new gcontext::text_engine_type(fake_loader, 0));
				ras.reset(new gcontext::rasterizer_type);
				ctx.reset(new gcontext(*surface, *renderer, *text_engine, make_vector(0, 0)));
			}

			test( MouseEnterSetsCursorToArrow )
			{
				// INIT / ACT
				root_container rc(cursor_manager_);
				container &c = rc;

				cursor_manager_->cursors[cursor_manager::arrow].reset(new cursor(10, 10, 1, 1));

				// ACT
				c.mouse_enter();

				// ASSERT
				assert_equal(cursor_manager_->cursors[cursor_manager::arrow], cursor_manager_->recently_set);
				assert_equal(1u, cursor_manager_->stack_level);
			}


			test( MouseLeavePopsCursorStack )
			{
				// INIT / ACT
				root_container rc(cursor_manager_);
				container &c = rc;
				auto lm = make_shared<mocks::logging_layout_manager>();
				view_location l = { 0, 0, 100, 100 };
				const auto v = make_shared<mocks::view>();
				auto mouse_left = false;

				lm->positions.push_back(l);
				cursor_manager_->cursors[cursor_manager::arrow].reset(new cursor(10, 10, 1, 1));
				c.set_layout(lm);
				c.add_view(v);
				c.resize(100, 100, nviews);
				c.mouse_enter();
				c.mouse_move(0, 10, 10);
				v->on_mouse_leave = [&] {
					mouse_left = true;

				// ASSERT
					assert_equal(1u, cursor_manager_->stack_level);
				};

				// ACT
				c.mouse_leave();

				// ASSERT
				assert_is_true(mouse_left);
				assert_equal(0u, cursor_manager_->stack_level);
			}


			test( SurfaceIsFilledWithBackgroundColorOnDraw )
			{
				// INIT / ACT
				root_container rc(cursor_manager_);
				container &c = rc;
				auto invalidates = 0;
				const auto lm = make_shared<mocks::logging_layout_manager>();
				const auto invalidate_conn = c.invalidate += [&] (const void *area) {
					assert_null(area);
					invalidates++;
				};
				view_location l = { 0, 0, 100, 20 };
				const auto v = make_shared<mocks::view>();
				auto drawn = 0;
				stylesheet_db ss;
	
				lm->positions.push_back(l);
				c.set_layout(lm);
				c.add_view(v);

				c.resize(100, 20, nviews);
				ss.set_color("background", agge::color::make(255, 128, 64));
				v->on_draw = [&] (const gcontext &) {
					drawn++;

				// ASSERT
					assert_equal(ref_rectangle(make_rect(0, 0, 100, 20), agge::color::make(255, 128, 64)), *surface);
				};

				// ACT
				rc.apply_styles(ss);
				c.draw(*ctx, ras);

				// ASSERT
				assert_equal(1, drawn);
				assert_equal(1, invalidates);

				// INIT
				c.resize(120, 50, nviews);
				ss.set_color("background", agge::color::make(255, 255, 255));
				v->on_draw = [&] (const gcontext &) {
					drawn++;

				// ASSERT
					assert_equal(ref_rectangle(make_rect(0, 0, 120, 50), agge::color::make(255, 255, 255)), *surface);
				};

				// ACT
				rc.apply_styles(ss);
				c.draw(*ctx, ras);

				// ASSERT
				assert_equal(2, drawn);
				assert_equal(2, invalidates);
			}
		end_test_suite
	}
}
