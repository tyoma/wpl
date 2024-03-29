#include <wpl/controls/range_slider.h>

#include "helpers.h"

#include <tests/common/Mockups.h>
#include <tests/common/helpers-visual.h>
#include <tests/common/predicates.h>

#include <ut/assert.h>
#include <ut/test.h>

using namespace agge;
using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace mocks
		{
			class slider_model : public wpl::sliding_window_model
			{
			public:
				virtual std::pair<double /*[min*/, double /*max)*/> get_range() const override
				{	return range;	}

				virtual std::pair<double /*[window_min*/, double /*window_width)*/> get_window() const override
				{	return window;	}

				virtual void scrolling(bool begins) override
				{
					if (on_scrolling)
						on_scrolling(begins);
				}

				virtual void set_window(double window_min, double window_width) override
				{
					if (on_scroll)
						on_scroll(window_min, window_width);
				}

				std::function<void (bool begins)> on_scrolling;
				std::function<void (double window_min, double window_width)> on_scroll;

			public:
				std::pair<double, double> range, window;
			};
		}

		namespace
		{
			class range_slider : public controls::range_slider_core
			{
			public:
				using controls::range_slider_core::descriptor;

			public:
				descriptor next_thumb;
				thumb_part next_hit_test;
				vector<point_r> hit_test_log;
				vector<descriptor> hit_test_states_log;
				mutable vector<descriptor> draw_log;

			private:
				virtual descriptor initialize(box_r /*box_*/) const override
				{	return next_thumb;	}

				virtual void draw(const descriptor &thumb, gcontext &/*ctx*/,
					gcontext::rasterizer_ptr &/*rasterizer_*/) const override
				{	draw_log.push_back(thumb);	}

				virtual thumb_part hit_test(const descriptor &thumb, point_r point_) override
				{
					hit_test_states_log.push_back(thumb);
					hit_test_log.push_back(point_);
					return next_hit_test;
				}
			};

			range_slider::descriptor make_descriptor(real_t y, real_t channel_near_x, real_t channel_far_x,
				real_t thumb_near_x, real_t thumb_far_x)
			{
				range_slider::descriptor d = {	y, {	channel_near_x, channel_far_x	}, {	thumb_near_x, thumb_far_x	}	};
				return d;
			}

			struct thumb_eq : eq
			{
				using eq::operator ();
				bool operator ()(const range_slider::descriptor &lhs, const range_slider::descriptor &rhs) const
				{
					return (*this)(lhs.y, rhs.y)
						&& (*this)(lhs.channel.near_x, rhs.channel.near_x)
						&& (*this)(lhs.channel.far_x, rhs.channel.far_x)
						&& (*this)(lhs.thumb.near_x, rhs.thumb.near_x)
						&& (*this)(lhs.thumb.far_x, rhs.thumb.far_x);
				}
			};
		}

		begin_test_suite( RangeSliderTests )

			mocks::font_loader fake_loader;
			unique_ptr<gcontext::surface_type> surface;
			unique_ptr<gcontext::renderer_type> renderer;
			unique_ptr<gcontext::text_engine_type> text_engine;
			gcontext::rasterizer_ptr rasterizer_;
			unique_ptr<gcontext> ctx;
			capture_source captured;


			init( Init )
			{
				surface.reset(new gcontext::surface_type(100, 100, 0));
				renderer.reset(new gcontext::renderer_type(1));
				text_engine.reset(new gcontext::text_engine_type(fake_loader, 0));
				rasterizer_.reset(new gcontext::rasterizer_type);
				ctx.reset(new gcontext(*surface, *renderer, *text_engine, make_vector(103, 71)));
			}


			test( MouseIsCapturedWhenHittingThumbArea )
			{
				// INIT
				auto slider = make_shared<range_slider>();
				shared_ptr<mouse_input> v = slider;
				auto m = make_shared<mocks::slider_model>();

				captured.attach_to(*v);
				slider->next_hit_test = controls::range_slider_core::part_near;
				slider->set_model(m);

				// ACT
				v->mouse_down(mouse_input::left, 0, 10, 13);

				// ASSERT
				point_r reference1[] = {
					{	10.0f, 13.0f	},
				};

				assert_equal(reference1, slider->hit_test_log);
				assert_not_null(captured.target());

				// ACT
				captured.target()->mouse_up(mouse_input::right, 0, 1000, 1000);

				// ASSERT
				assert_not_null(captured.target());

				// ACT
				captured.target()->mouse_up(mouse_input::left, 0, 1000, 1000);

				// ASSERT
				assert_null(captured.target());

				// INIT
				slider->next_hit_test = controls::range_slider_core::part_far;

				// ACT
				v->mouse_down(mouse_input::left, 0, 101, 1311);

				// ASSERT
				point_r reference2[] = {
					{	10.0f, 13.0f	},
					{	101.0f, 1311.0f	},
				};

				assert_equal(reference2, slider->hit_test_log);
				assert_not_null(captured.target());

				// ACT
				captured.target()->mouse_up(mouse_input::left, 0, 0, 0);

				// ASSERT
				assert_null(captured.target());

				// INIT
				slider->next_hit_test = controls::range_slider_core::part_shaft;

				// ACT
				v->mouse_down(mouse_input::left, 0, 191, 3119);

				// ASSERT
				point_r reference3[] = {
					{	10.0f, 13.0f	},
					{	101.0f, 1311.0f	},
					{	191.0f, 3119.0f	},
				};

				assert_equal(reference3, slider->hit_test_log);
				assert_not_null(captured.target());
			}


			test( MouseIsNotCapturedIfNoThumbIsHit )
			{
				// INIT
				auto slider = make_shared<range_slider>();
				shared_ptr<view> v = slider;
				captured.attach_to(*v);
				slider->next_hit_test = controls::range_slider_core::part_none;

				// ACT
				v->mouse_down(mouse_input::left, 0, 10, 13);

				// ASSERT
				assert_null(captured.target());
			}


			test( ThumbCoodinatesAreRecalculatedOnModelInvalidateAndBoxChange )
			{
				// INIT
				auto slider = make_shared<range_slider>();
				shared_ptr<visual> v = slider;
				auto m = make_shared<mocks::slider_model>();

				m->range = make_pair(10.0, 100.0);
				m->window = make_pair(10.0, 100.0);

				slider->next_thumb = make_descriptor(12.3f, 5.3f, 107.0f, 0.0f, 0.0f);
				resize(*slider, 300, 170);
				slider->set_model(m);

				// ACT
				v->draw(*ctx, rasterizer_);

				// ASSERT
				range_slider::descriptor reference1[] = {
					make_descriptor(12.3f, 5.3f, 107.0f, 5.3f, 107.0f),
				};

				assert_equal_pred(reference1, slider->draw_log, thumb_eq());

				// INIT / ACT
				m->window = make_pair(17.7, 73.1);
				m->invalidate(false);

				// ACT
				v->draw(*ctx, rasterizer_);

				// ASSERT
				range_slider::descriptor reference2[] = {
					make_descriptor(12.3f, 5.3f, 107.0f, 5.3f, 107.0f),
					make_descriptor(12.3f, 5.3f, 107.0f, 13.131f, 87.474f),
				};

				assert_equal_pred(reference2, slider->draw_log, thumb_eq());

				// INIT / ACT
				m->range = make_pair(0.0, 100.0);
				m->invalidate(true);

				// ACT
				v->draw(*ctx, rasterizer_);

				// ASSERT
				range_slider::descriptor reference3[] = {
					make_descriptor(12.3f, 5.3f, 107.0f, 5.3f, 107.0f),
					make_descriptor(12.3f, 5.3f, 107.0f, 13.131f, 87.474f),
					make_descriptor(12.3f, 5.3f, 107.0f, 23.301f, 97.644f),
				};

				assert_equal_pred(reference3, slider->draw_log, thumb_eq());

				// INIT
				slider->next_thumb = make_descriptor(10.0f, 10.0f, 90.0f, 0.0f, 0.0f);
				resize(*slider, 300, 170);

				// ACT
				v->draw(*ctx, rasterizer_);

				// ASSERT
				range_slider::descriptor reference4[] = {
					make_descriptor(12.3f, 5.3f, 107.0f, 5.3f, 107.0f),
					make_descriptor(12.3f, 5.3f, 107.0f, 13.131f, 87.474f),
					make_descriptor(12.3f, 5.3f, 107.0f, 23.301f, 97.644f),
					make_descriptor(10.0f, 10.0f, 90.0f, 24.160f, 82.640f),
				};

				assert_equal_pred(reference4, slider->draw_log, thumb_eq());
			}


			test( CalculatedStateIsPassedToHitTest )
			{
				// INIT
				auto slider = make_shared<range_slider>();
				shared_ptr<mouse_input> v = slider;
				auto m = make_shared<mocks::slider_model>();

				captured.attach_to(*v);
				m->range = make_pair(10.0, 100.0);
				m->window = make_pair(10.0, 100.0);
				slider->next_thumb = make_descriptor(12.3f, 5.3f, 107.0f, 0.0f, 0.0f);
				slider->next_hit_test = range_slider::part_near;
				resize(*slider, 300, 170);
				slider->set_model(m);

				// ACT
				v->mouse_down(mouse_input::left, 0, 10, 13);

				// ASSERT
				range_slider::descriptor reference1[] = {
					make_descriptor(12.3f, 5.3f, 107.0f, 5.3f, 107.0f),
				};

				assert_equal_pred(reference1, slider->hit_test_states_log, thumb_eq());

				// INIT
				captured.target()->mouse_up(mouse_input::left, 0, 10, 13);
				m->window = make_pair(31.4, 29.1);
				m->invalidate(false);

				// ACT
				v->mouse_down(mouse_input::left, 0, 10, 13);

				// ASSERT
				range_slider::descriptor reference2[] = {
					make_descriptor(12.3f, 5.3f, 107.0f, 5.3f, 107.0f),
					make_descriptor(12.3f, 5.3f, 107.0f, 27.064f, 56.659f),
				};

				assert_equal_pred(reference2, slider->hit_test_states_log, thumb_eq());
			}


			test( DraggingLeftSliderUpdatesLowerBoundOnly )
			{
				// INIT
				auto slider = make_shared<range_slider>();
				shared_ptr<mouse_input> v = slider;
				vector< pair<double, double> > log;
				auto m = make_shared<mocks::slider_model>();

				captured.attach_to(*v);
				m->range = make_pair(0.0, 100.0);
				m->window = make_pair(10.3, 53.0);
				m->on_scroll = [&] (double window_min, double window_width) {
					log.push_back(make_pair(window_min, window_width));
				};
				slider->next_thumb = make_descriptor(12.3f, 5.3f, 107.0f, 0.0f, 0.0f);
				slider->next_hit_test = range_slider::part_near;
				slider->set_model(m);
				resize(*slider, 300, 170); // thumb: 15.775+69.676
				captured.attach_to(*v);

				// ACT
				v->mouse_down(mouse_input::left, 0, 16, 12);
				m->range = make_pair(1.3, 1.0); // Try to confuse slider - set weird range.
				captured.target()->mouse_move(0, 20, 1000); // +4px == 3.9331

				// ASSERT
				pair<double, double> reference1[] = {
					make_pair(14.233, 49.067),
				};

				assert_equal_pred(reference1, log, eq());

				// ACT
				captured.target()->mouse_move(0, 21, 7);
				captured.target()->mouse_move(0, 0, 10);

				// ASSERT
				pair<double, double> reference2[] = {
					make_pair(14.233, 49.067),
					make_pair(15.216, 48.084),
					make_pair(-5.4325, 68.733),
				};

				assert_equal_pred(reference2, log, eq());
			}


			test( DraggingRightSliderUpdatesUpperBoundOnly )
			{
				// INIT
				auto slider = make_shared<range_slider>();
				shared_ptr<mouse_input> v = slider;
				vector< pair<double, double> > log;
				auto m = make_shared<mocks::slider_model>();

				captured.attach_to(*v);
				m->range = make_pair(0.0, 80.0);
				m->window = make_pair(10.3, 53.0);
				m->on_scroll = [&] (double window_min, double window_width) {
					log.push_back(make_pair(window_min, window_width));
				};
				slider->next_thumb = make_descriptor(30.0f, 5.3f, 105.3f, 0.0f, 0.0f);
				slider->next_hit_test = range_slider::part_shaft;
				slider->set_model(m);
				resize(*slider, 300, 170); // thumb: 15.775+69.676
				captured.attach_to(*v);

				// ACT
				v->mouse_down(mouse_input::left, 0, 70, 30);
				m->range = make_pair(1.3, 1.0); // Try to confuse slider - set weird range.
				captured.target()->mouse_move(0, 81, 1000); // +11px == 8.8

				// ASSERT
				pair<double, double> reference1[] = {
					make_pair(19.100, 53.000),
				};

				assert_equal_pred(reference1, log, eq());

				// ACT
				captured.target()->mouse_move(0, 13, 1000);
				captured.target()->mouse_move(0, 140, 1000);

				// ASSERT
				pair<double, double> reference2[] = {
					make_pair(19.100, 53.000),
					make_pair(-35.300, 53.000),
					make_pair(66.300, 53.000),
				};

				assert_equal_pred(reference2, log, eq());
			}


			test( SettingModelInvalidatesView )
			{
				// INIT
				auto invalidates = 0;
				auto slider = make_shared<range_slider>();
				shared_ptr<view> v = slider;
				auto m = make_shared<mocks::slider_model>();
				auto c = v->invalidate += [&] (const void *window) {
					assert_null(window);
					invalidates++;
				};

				m->range = make_pair(0.0, 10.0);
				m->window = make_pair(2.3, 1.7);
				slider->next_thumb = make_descriptor(10.0f, 0.0f, 100.0f, 0.0f, 0.0f);
				resize(*slider, 100, 100);

				// ACT
				slider->set_model(m);

				// ASSERT
				assert_equal(1, invalidates);

				// ACT
				slider->set_model(nullptr);

				// ASSERT
				assert_equal(2, invalidates);

				// ACT
				m->invalidate(false);

				// ASSERT
				assert_equal(2, invalidates);
			}


			test( ViewIsInvalidatesOnModelInvalidate )
			{
				// INIT
				auto invalidates = 0;
				auto slider = make_shared<range_slider>();
				shared_ptr<view> v = slider;
				auto m = make_shared<mocks::slider_model>();
				auto c = v->invalidate += [&] (const void *window) {
					assert_null(window);
					invalidates++;
				};

				m->range = make_pair(0.0, 10.0);
				m->window = make_pair(2.3, 1.7);
				slider->next_thumb = make_descriptor(10.0f, 0.0f, 100.0f, 0.0f, 0.0f);
				resize(*slider, 100, 100);
				slider->set_model(m);
				invalidates = 0;

				// ACT
				m->invalidate(false);

				// ASSERT
				assert_equal(1, invalidates);

				// ACT
				m->invalidate(false);
				m->invalidate(true);

				// ASSERT
				assert_equal(3, invalidates);
			}


			test( ScrollingBracketsAreNotifiedUponOnDragStart )
			{
				// INIT
				auto slider = make_shared<range_slider>();
				shared_ptr<view> v = slider;
				auto m = make_shared<mocks::slider_model>();
				vector<bool> log;

				slider->next_hit_test = range_slider::part_near;
				resize(*slider, 100, 100);
				slider->set_model(m);
				captured.attach_to(*v);
				m->on_scrolling = [&] (bool begins) {	log.push_back(begins);	};

				// ACT
				v->mouse_down(mouse_input::left, 0, 0, 0);

				// ASSERT
				assert_equal(plural + true, log);

				// ACT
				captured.target()->mouse_up(mouse_input::left, 0, 0, 0);

				// ASSERT
				assert_equal(plural + true + false, log);

				// INIT
				slider->next_hit_test = range_slider::part_shaft;

				// ACT
				v->mouse_down(mouse_input::left, 0, 0, 0);

				// ASSERT
				assert_equal(plural + true + false + true, log);

				// ACT
				captured.target()->mouse_up(mouse_input::left, 0, 0, 0);

				// ASSERT
				assert_equal(plural + true + false + true + false, log);

				// INIT
				slider->next_hit_test = range_slider::part_far;

				// ACT
				v->mouse_down(mouse_input::left, 0, 0, 0);

				// ASSERT
				assert_equal(plural + true + false + true + false + true, log);

				// ACT
				captured.target()->mouse_up(mouse_input::left, 0, 0, 0);

				// ASSERT
				assert_equal(plural + true + false + true + false + true + false, log);

				// INIT
				slider->next_hit_test = range_slider::part_none;

				// ACT
				v->mouse_down(mouse_input::left, 0, 0, 0);

				// ASSERT
				assert_equal(plural + true + false + true + false + true + false, log);
			}
		end_test_suite
	}
}
