#include <wpl/controls/scroller.h>

#include "helpers-visual.h"
#include "Mockups.h"
#include "MockupsScroller.h"
#include "predicates.h"

#include <string.h>
#include <functional>
#include <ut/assert.h>
#include <ut/test.h>

using namespace agge;
using namespace std;
using namespace placeholders;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			controls::scroller::thumb make_thumb(double width, double lbound, double ubound)
			{
				controls::scroller::thumb t = {
					true,
					static_cast<real_t>(width),
					static_cast<real_t>(lbound),
					static_cast<real_t>(ubound)
				};

				return t;
			}

			struct eq2 : eq
			{
				eq2(double tolerance = 0.0001)
					: eq(tolerance)
				{	}

				using eq::operator();

				bool operator ()(controls::scroller::thumb lhs, controls::scroller::thumb rhs) const
				{
					return lhs.active == rhs.active && (*this)(lhs.width, rhs.width) && (*this)(lhs.lbound, rhs.lbound)
						&& (*this)(lhs.ubound, rhs.ubound);
				}
			};
		}

		begin_test_suite( ScrollerTests )
			typedef vector< pair<bool, agge::rect_i> > invalidations_log;

			mocks::font_loader fake_loader;

			static void invalidate_cb(invalidations_log *log, const agge::rect_i *w)
			{	log->push_back(w ? make_pair(true, *w) : make_pair(false, agge::rect_i()));	}

			function<void (const agge::rect_i *window)> log_invalidates(invalidations_log &log)
			{	return bind(&invalidate_cb, &log, _1);	}


			test( ScrollerIsNotTabstoppable )
			{
				// INIT / ACT
				controls::scroller sh(controls::scroller::horizontal);
				controls::scroller sv(controls::scroller::vertical);

				// ACT / ASSERT
				assert_is_false(provides_tabstoppable_view(sh));
				assert_is_false(provides_tabstoppable_view(sv));
			}

			test( ModellessScrollerDoesNothingOnVisualEvents )
			{
				// INIT / ACT
				controls::scroller s(controls::scroller::horizontal);
				slot_connection iconn = s.invalidate += [] (const agge::rect_i *) { assert_is_true(false); };
				gcontext::surface_type surface(1000, 1000, 0);
				gcontext::renderer_type ren(1);
				gcontext::text_engine_type text_engine(fake_loader, 0);
				gcontext::rasterizer_ptr ras(new gcontext::rasterizer_type);
				gcontext ctx(surface, ren, text_engine, make_vector(103, 71));

				// ACT / ASSERT
				s.draw(ctx, ras);

				// ASSERT
				assert_equal(0, ras->height()); // a way to check emptiness

				// ACT / ASSERT (ok to call with zero dimensions)
				resize(s, 100, 0);
				resize(s, 0, 100);
				resize(s, 0, 0);
			}


			test( ModellessScrollerDoesNothingOnMouseEvents )
			{
				// INIT / ACT
				controls::scroller s(controls::scroller::horizontal);
				slot_connection c = s.capture += [] (shared_ptr<void> &) { assert_is_true(false); };

				resize(s, 100, 10);

				// ACT / ASSERT (no crashes)
				s.mouse_down(mouse_input::left, 0, 0, 5);
				s.mouse_up(mouse_input::left, 0, 0, 5);
				s.mouse_down(mouse_input::left, 0, 50, 5);
				s.mouse_up(mouse_input::left, 0, 50, 5);
				s.mouse_down(mouse_input::left, 0, 99, 5);
				s.mouse_up(mouse_input::left, 0, 99, 5);
			}


			test( NoInvalidationOrDrawingIsMadeOnZeroDimensionResize )
			{
				// INIT
				shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
				controls::scroller sh(controls::scroller::horizontal), sv(controls::scroller::vertical);
				gcontext::surface_type surface(1000, 1000, 0);
				gcontext::renderer_type ren(1);
				gcontext::text_engine_type text_engine(fake_loader, 0);
				gcontext::rasterizer_ptr ras(new gcontext::rasterizer_type);
				gcontext ctx(surface, ren, text_engine, make_vector(103, 71));

				m->range = make_pair(0, 100);

				resize(sh, 100, 20);
				sh.set_model(m);
				resize(sv, 20, 100);
				sv.set_model(m);

				slot_connection iconnh = sh.invalidate += [] (const agge::rect_i *) { assert_is_true(false); };
				slot_connection iconnv = sv.invalidate += [] (const agge::rect_i *) { assert_is_true(false); };

				// ACT / ASSERT
				resize(sh, 0, 10);
				resize(sv, 10, 0);
				sh.draw(ctx, ras);
				sv.draw(ctx, ras);

				// ASSERT
				assert_equal(0, ras->height());

				// ACT / ASSERT
				resize(sh, 100, 0);
				resize(sv, 0, 100);
				sh.draw(ctx, ras);
				sv.draw(ctx, ras);

				// ASSERT
				assert_equal(0, ras->height());
			}


			test( NothingIsDrawnIfNoModelSetIfModelIsEmptyOrIfWindowIsNotSmallerThanTheRange )
			{
				// INIT / ACT
				controls::scroller s(controls::scroller::horizontal);
				gcontext::surface_type surface(1000, 1000, 0);
				gcontext::renderer_type ren(1);
				gcontext::text_engine_type text_engine(fake_loader, 0);
				gcontext::rasterizer_ptr ras(new gcontext::rasterizer_type);
				gcontext ctx(surface, ren, text_engine, agge::zero());

				memset(surface.row_ptr(0), 0, sizeof(gcontext::surface_type::pixel) * 1000 * 1000);
				resize(s, 100, 10);

				shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
				m->range = make_pair(0, 0);
				m->window = make_pair(0, 0);

				// ACT / ASSERT
				s.draw(ctx, ras);
				s.set_model(m);
				s.draw(ctx, ras);
				m->range = make_pair(0, 10);
				m->window = make_pair(3, 100);
				s.draw(ctx, ras);

				// ASSERT
				for (agge::count_t i = 0, count = surface.height(); i != count; ++i)
					assert_equal(surface.row_ptr(i) + 1000, find_if(surface.row_ptr(i), surface.row_ptr(i) + 1000,
						[] (const gcontext::surface_type::pixel &p) {
						return p.components[0] || p.components[1] || p.components[2] || p.components[3];
					}));
			}


			test( ScrollerIsInvalidatedOnModelSetting )
			{
				// INIT
				invalidations_log ilogh, ilogv;
				shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
				controls::scroller sh(controls::scroller::horizontal), sv(controls::scroller::vertical);
				slot_connection c1 = sh.invalidate += log_invalidates(ilogh);
				slot_connection c2 = sv.invalidate += log_invalidates(ilogv);

				// ACT
				sh.set_model(m);

				// ASSERT
				assert_equal(1u, ilogh.size());
				assert_is_false(ilogh[0].first);

				// ACT
				sv.set_model(m);

				// ASSERT
				assert_equal(1u, ilogv.size());
				assert_is_false(ilogv[0].first);
			}


			test( ThumbIsInactiveIfNoModelIsNotSet )
			{
				// INIT
				controls::scroller sh(controls::scroller::horizontal), sv(controls::scroller::vertical);

				// ACT / ASSERT
				assert_is_false(sh.get_thumb().active);
				assert_is_false(sv.get_thumb().active);
			}


			test( ThumbIsInactiveIfWindowIsNotSmallerThanTheRange )
			{
				// INIT
				shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
				controls::scroller s(controls::scroller::horizontal);

				s.set_model(m);
				m->range = make_pair(10.1, 100.7);
				m->window = make_pair(10.1, 100.7);

				// ACT / ASSERT
				assert_is_false(s.get_thumb().active);

				// INIT
				m->window = make_pair(30.1, 100.7);

				// ACT / ASSERT
				assert_is_false(s.get_thumb().active);

				// INIT
				m->range = make_pair(3.1, 7.1);
				m->window = make_pair(3.1, 10.7);

				// ACT / ASSERT
				assert_is_false(s.get_thumb().active);

				// INIT
				m->window = make_pair(3.1, 6.7);

				// ACT / ASSERT
				assert_is_true(s.get_thumb().active);
			}


			test( ThumbCoordinatesAreCalculatedFromScrollerExtentAndModelRangeAndModelWindowNoLimitApplied )
			{
				// INIT
				shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
				controls::scroller sh(controls::scroller::horizontal), sv(controls::scroller::vertical);

				m->range = make_pair(10.1, 100.7);
				m->window = make_pair(50.3, 14.5);
				resize(sh, 94, 10);
				resize(sv, 13, 194);

				sh.set_model(m);
				sv.set_model(m);

				// ACT / ASSERT
				assert_equal_pred(make_thumb(7, 38.53, 50.63), sh.get_thumb(), eq2(0.0005));
				assert_equal_pred(make_thumb(9.1, 78.76, 104.8), sv.get_thumb(), eq2(0.0005));

				// INIT
				m->window = make_pair(10.1, 25);

				// ACT / ASSERT
				assert_equal_pred(make_thumb(7, 5, 25.85), sh.get_thumb(), eq2(0.0005));
				assert_equal_pred(make_thumb(9.1, 6.5, 51.44), sv.get_thumb(), eq2(0.0005));

				// INIT
				m->range = make_pair(-100, 200);

				// ACT / ASSERT
				assert_equal_pred(make_thumb(7, 51.24, 61.74), sh.get_thumb(), eq2(0.0005));
				assert_equal_pred(make_thumb(9.1, 106.1, 128.8), sv.get_thumb(), eq2(0.0005));
			}


			test( PageScrollAttemptIsMadeWhenClickingOutsideThumb )
			{
				// INIT
				invalidations_log ilog;
				vector< pair<double, double> > log;
				shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
				controls::scroller sh(controls::scroller::horizontal), sv(controls::scroller::vertical);

				m->range = make_pair(0, 500);
				m->window = make_pair(50, 27);
				m->on_scrolling = [] (bool) { assert_is_true(false); };
				m->on_scroll = [&] (double m, double w) { log.push_back(make_pair(m, w)); };

				resize(sh, 110, 10);
				sh.set_model(m);
				resize(sv, 1, 251);
				sv.set_model(m);

				slot_connection connections[] = {
					sh.invalidate += log_invalidates(ilog),
					sh.capture += [] (shared_ptr<void> &/*handle*/) { assert_is_true(false); },
					sv.invalidate += log_invalidates(ilog),
					sv.capture += [] (shared_ptr<void> &/*handle*/) { assert_is_true(false); },
				};
				connections;

				// ACT
				sh.mouse_down(mouse_input::left, 0, 0, -200 /*ignored*/); // one page less

				// ASSERT
				pair<double, double> reference1[] = { make_pair(23, 27), };

				assert_is_empty(ilog);
				assert_equal_pred(reference1, log, eq2());

				// INIT
				log.clear();

				// ACT
				sh.mouse_down(mouse_input::left, 0, 9 - 5, 0); // one page less

				// ASSERT
				assert_equal_pred(reference1, log, eq2());

				// INIT
				log.clear();

				// ACT
				sh.mouse_down(mouse_input::left, 0, 16 + 5, 0); // one page more

				// ASSERT
				pair<double, double> reference2[] = { make_pair(77, 27), };

				assert_equal_pred(reference2, log, eq2());

				// INIT
				log.clear();

				// ACT
				sh.mouse_down(mouse_input::left, 0, 99, 200 /*ignored*/); // one page more

				// ASSERT
				assert_equal_pred(reference2, log, eq2());

				// INIT
				log.clear();

				// ACT
				sv.mouse_down(mouse_input::left, 0, -100 /*ignored*/, 0); // one page less

				// ASSERT
				assert_is_empty(ilog);
				assert_equal_pred(reference1, log, eq2());

				// INIT
				log.clear();

				// ACT
				sv.mouse_down(mouse_input::left, 0, 0, 24 - 1); // one page less

				// ASSERT
				assert_equal_pred(reference1, log, eq2());

				// INIT
				log.clear();

				// ACT
				sv.mouse_down(mouse_input::left, 0, 0, 39 + 1); // one page more

				// ASSERT
				assert_equal_pred(reference2, log, eq2());

				// INIT
				log.clear();

				// ACT
				sv.mouse_down(mouse_input::left, 0, 100 /*ignored*/, 249); // one page more

				// ASSERT
				assert_equal_pred(reference2, log, eq2());

				// INIT
				m->range = make_pair(21.5, 200);
				m->window = make_pair(51.5, 20);
				log.clear();

				// ACT
				sh.mouse_down(mouse_input::left, 0, 14 - 5, -10); // one page less

				// ASSERT
				pair<double, double> reference3[] = { make_pair(31.5, 20), };

				assert_equal_pred(reference3, log, eq2());

				// INIT
				log.clear();

				// ACT
				sh.mouse_down(mouse_input::left, 0, 25 + 5, 10); // one page more

				// ASSERT
				pair<double, double> reference4[] = { make_pair(71.5, 20), };

				assert_equal_pred(reference4, log, eq2());
			}


			test( PageScrollAttemptIsLimitedToTheRangeBoundaries )
			{
				// INIT
				vector< pair<double, double> > log;
				shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
				controls::scroller s(controls::scroller::horizontal);

				m->range = make_pair(110.7, 150.1);
				m->window = make_pair(125, 20);
				m->on_scroll = [&] (double m, double w) { log.push_back(make_pair(m, w)); };

				resize(s, 100, 10);
				s.set_model(m);

				slot_connection c = s.capture += [] (shared_ptr<void> &/*handle*/) { assert_is_true(false); };

				// ACT
				s.mouse_down(mouse_input::left, 0, 0, 0); // one page less

				// ASSERT
				pair<double, double> reference1[] = { make_pair(110.7, 20), };

				assert_equal_pred(reference1, log, eq2());

				// INIT
				log.clear();
				m->window = make_pair(230, 21);

				// ACT
				s.mouse_down(mouse_input::left, 0, 99, 0); // one page more

				// ASSERT
				pair<double, double> reference2[] = { make_pair(239.8, 21), };

				assert_equal_pred(reference2, log, eq2());
			}


			test( ModelInvalidationLeadsToControlInvalidation )
			{
				shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
				controls::scroller s(controls::scroller::horizontal);
				invalidations_log ilog;

				resize(s, 100, 10);
				s.set_model(m);

				slot_connection c = s.invalidate += log_invalidates(ilog);

				// ACT
				m->invalidated();

				// ASSERT
				assert_equal(1u, ilog.size());
				assert_is_false(ilog[0].first);

				// ACT
				s.set_model(shared_ptr<scroll_model>());

				// ASSERT
				assert_equal(2u, ilog.size());

				// ACT
				m->invalidated();

				// ASSERT
				assert_equal(2u, ilog.size());
			}


			test( NothingHappensOnClickWhenThumbIsInactive )
			{
				// INIT
				mocks::capture_provider cp;
				invalidations_log ilog;
				shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
				controls::scroller s(controls::scroller::vertical);

				m->range = make_pair(0, 56);
				m->window = make_pair(0, 57);
				resize(s, 30, 280);
				s.set_model(m);

				// ASSERT
				m->on_scrolling = [&] (bool) { assert_is_true(false);	};
				m->on_scroll = [&] (double, double) { assert_is_true(false); };

				// ACT
				s.mouse_down(mouse_input::left, 0, 15, 140);
				s.mouse_down(mouse_input::left, 0, 15, 140);

				// ASSERT
				assert_is_empty(cp.log);

				// ACT
				s.mouse_move(mouse_input::left, 15, 138);

				// ASSERT
				assert_is_empty(cp.log);

				// ACT
				s.mouse_up(mouse_input::left, 0, 15, 138);

				// ASSERT
				assert_is_empty(cp.log);
			}


			test( LButtonCycleWithinThumbCapturesReleasesMouse )
			{
				// INIT
				mocks::capture_provider cp;
				invalidations_log ilog;
				shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
				controls::scroller sh(controls::scroller::horizontal), sv(controls::scroller::vertical);
				bool scrolling = false;

				m->range = make_pair(0, 500);
				m->window = make_pair(50, 40);
				m->on_scrolling = [&] (bool begins) {
					scrolling = begins;
				};
				m->on_scroll = [&] (double, double) { assert_is_true(false); };

				resize(sh, 110, 10);
				sh.set_model(m);
				resize(sv, 30, 280);
				sv.set_model(m);

				cp.add_view(sh);
				cp.add_view(sv);

				// ACT
				sh.mouse_down(mouse_input::left, 0, 10 + 5 /*lower draggable*/, 5);

				// ASSERT
				pair<view *, bool> reference1[] = { make_pair(&sh, true), };

				assert_is_true(scrolling);
				assert_equal(reference1, cp.log);

				// INIT
				cp.log.clear();
				scrolling = false;

				// ACT
				sv.mouse_down(mouse_input::left, 0, 15, 25 + 15 /*lower draggable*/);

				// ASSERT
				pair<view *, bool> reference2[] = { make_pair(&sv, true), };

				assert_is_true(scrolling);
				assert_equal(reference2, cp.log);

				// INIT
				m->on_scrolling = [&] (bool begins) {
					scrolling = begins;
					if (!begins)
					{
						assert_is_false(cp.log.empty());
						assert_is_false(cp.log.back().second);
					}
				};

				// ACT
				sh.mouse_up(mouse_input::left, 0, 10, 5);

				// ASSERT
				pair<view *, bool> reference3[] = { make_pair(&sv, true), make_pair(&sh, false), };

				assert_is_false(scrolling);
				assert_equal(reference3, cp.log);

				// INIT
				scrolling = true;

				// ACT
				sv.mouse_up(mouse_input::left, 0, 15, 25);

				// ASSERT
				pair<view *, bool> reference4[] = {
					make_pair(&sv, true), make_pair(&sh, false), make_pair(&sv, false),
				};

				assert_is_false(scrolling);
				assert_equal(reference4, cp.log);
			}


			test( MovingMouseWhileNotCapturedDoesNothing )
			{
				// INIT
				vector< pair<double, double> > log;
				shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
				controls::scroller s(controls::scroller::horizontal);

				m->range = make_pair(10, 101);
				m->window = make_pair(50, 20);
				m->on_scroll = [&] (double, double) { assert_is_true(false); };

				resize(s, 100, 10);
				s.set_model(m);

				// ACT / ASSERT
				s.mouse_move(mouse_input::left, 49, 100);
				s.mouse_move(mouse_input::left, 51, 100);
			}


			test( MovingMouseWhileCapturedMovesTheWindow )
			{
				// INIT
				mocks::capture_provider cp;
				vector< pair<double, double> > log;
				shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
				controls::scroller sh(controls::scroller::horizontal), sv(controls::scroller::vertical);

				m->range = make_pair(10, 101);
				m->window = make_pair(50, 20);
				m->on_scroll = [&] (double m, double w) { log.push_back(make_pair(m, w)); };

				resize(sh, 110, 10);
				cp.add_view(sh);
				sh.set_model(m);
				resize(sv, 10, 310);
				cp.add_view(sv);
				sv.set_model(m);

				sh.mouse_down(mouse_input::left, 0, 50 + 5, 5);
				sv.mouse_down(mouse_input::left, 0, 5, 150 + 5);

				assert_equal(2u, cp.log.size());

				// ACT
				m->window = make_pair(52, 21); // try to spoil the results - make sure the initial window is stored.
				sh.mouse_move(mouse_input::left, 49 + 5, 100);

				// ASSERT
				pair<double, double> reference1[] = { make_pair(50 - 1.01, 20), };

				assert_equal_pred(reference1, log, eq2());

				// ACT
				m->window = make_pair(12, 1);
				sh.mouse_move(mouse_input::left, 40 + 5, 100);

				// ASSERT
				pair<double, double> reference2[] = {
					make_pair(50 - 1.01, 20),
					make_pair(50 - 10.1, 20),
				};

				assert_equal_pred(reference2, log, eq2());

				// ACT
				sh.mouse_move(mouse_input::left, 61 + 5, -10);

				// ASSERT
				pair<double, double> reference3[] = {
					make_pair(50 - 1.01, 20),
					make_pair(50 - 10.1, 20),
					make_pair(50 + 1.01 * 11, 20),
				};

				assert_equal_pred(reference3, log, eq2());

				// ACT
				sv.mouse_move(mouse_input::left, 5, 152 + 5);

				// ASSERT
				pair<double, double> reference4[] = {
					make_pair(50 - 1.01, 20),
					make_pair(50 - 10.1, 20),
					make_pair(50 + 1.01 * 11, 20),
					make_pair(50 + 2.0 / 300.0 * 101.0, 20),
				};

				assert_equal_pred(reference4, log, eq2());
			}


			test( AlienMouseScrollsDoNotUpdateTheModel )
			{
				// INIT
				shared_ptr<mocks::scroll_model> mh(new mocks::scroll_model), mv(new mocks::scroll_model);
				controls::scroller sh(controls::scroller::horizontal), sv(controls::scroller::vertical);

				mh->range = make_pair(10, 101);
				mh->window = make_pair(50, 10);
				mh->on_scroll = [&] (double, double) {	assert_is_true(false);	};
				resize(sh, 100, 10);
				sh.set_model(mh);

				mv->range = make_pair(0, 10);
				mv->window = make_pair(5, 2);
				mv->on_scroll = [&] (double, double) {	assert_is_true(false);	};
				resize(sv, 10, 100);
				sv.set_model(mv);

				// ACT / ASSERT
				sh.mouse_scroll(0, 0, 0, 0, -10);
				sh.mouse_scroll(0, 0, 0, 0, 9);
				sv.mouse_scroll(0, 0, 0, -10, 0);
				sv.mouse_scroll(0, 0, 0, 9, 0);
			}


			test( MouseScrollsUpdatesTheModel )
			{
				// INIT
				vector< pair<double, double> > logh, logv;
				shared_ptr<mocks::scroll_model> mh(new mocks::scroll_model), mv(new mocks::scroll_model);
				controls::scroller sh(controls::scroller::horizontal), sv(controls::scroller::vertical);

				mh->range = make_pair(10, 101);
				mh->window = make_pair(50, 10);
				mh->on_scroll = [&] (double m, double w) {
					logh.push_back(make_pair(m, w));
					mh->window = make_pair(m, w);
				};
				resize(sh, 100, 10);
				sh.set_model(mh);

				mv->range = make_pair(0, 10);
				mv->window = make_pair(5, 2);
				mv->on_scroll = [&] (double m, double w) {
					logv.push_back(make_pair(m, w));
					mv->window = make_pair(m, w);
				};
				resize(sv, 10, 100);
				sv.set_model(mv);

				// ACT
				sh.mouse_scroll(0, 0, 0, -31, 0);
				sv.mouse_scroll(0, 0, 0, 0, -3);

				// ASSERT
				pair<double, double> reference1[] = { make_pair(81, 10), };
				pair<double, double> reference2[] = { make_pair(8, 2), };

				assert_equal_pred(reference1, logh, eq());
				assert_equal_pred(reference2, logv, eq());

				// ACT
				sh.mouse_scroll(0, 0, 0, 20, 0);
				sv.mouse_scroll(0, 0, 0, 0, 1);

				// ASSERT
				pair<double, double> reference3[] = { make_pair(81, 10), make_pair(61, 10), };
				pair<double, double> reference4[] = { make_pair(8, 2), make_pair(7, 2), };

				assert_equal_pred(reference3, logh, eq());
				assert_equal_pred(reference4, logv, eq());
			}


			test( MouseScrollsGeneratesScrollBeginScrollEndEvents )
			{
				// INIT
				vector<int> events;
				shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
				controls::scroller s(controls::scroller::horizontal);

				m->range = make_pair(10, 101);
				m->window = make_pair(50, 10);
				m->on_scrolling = [&] (bool start) {	events.push_back(start ? 1 : 3);	};
				m->on_scroll = [&] (double, double) {	events.push_back(2);	};
				resize(s, 100, 10);
				s.set_model(m);

				// ACT
				s.mouse_scroll(0, 0, 0, -31, 0);

				// ASSERT
				int reference1[] = { 1, 2, 3, };

				assert_equal(reference1, events);

				// INIT
				events.clear();

				// ACT
				s.mouse_scroll(0, 0, 0, 2, 0);
				s.mouse_scroll(0, 0, 0, 3, 0);
				s.mouse_scroll(0, 0, 0, 0, 1);

				// ASSERT
				int reference2[] = { 1, 2, 3, 1, 2, 3, };

				assert_equal(reference2, events);
			}


			test( MouseScrollIsDoneInModelProvidedIncrements )
			{
				// INIT
				vector< pair<double, double> > log;
				shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
				controls::scroller s(controls::scroller::vertical);

				m->range = make_pair(10, 101);
				m->window = make_pair(50, 10);
				m->on_scroll = [&] (double m, double w) {	log.push_back(make_pair(m, w));	};
				resize(s, 100, 10);
				s.set_model(m);

				m->increment = 3;

				// ACT
				s.mouse_scroll(0, 0, 0, 0, -10);

				// ASSERT
				pair<double, double> reference1[] = { make_pair(80, 10), };

				assert_equal_pred(reference1, log, eq());

				// INIT
				m->increment = 5;
				log.clear();

				// ACT
				s.mouse_scroll(0, 0, 0, 0, 3);

				// ASSERT
				pair<double, double> reference2[] = { make_pair(35, 10), };

				assert_equal_pred(reference2, log, eq());
			}

		end_test_suite
	}
}
