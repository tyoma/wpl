#include <wpl/ui/scroller.h>

#include "MockupsScroller.h"
#include "TestHelpers.h"

#include <functional>
#include <ut/assert.h>
#include <ut/test.h>

using namespace agge;
using namespace std;
using namespace placeholders;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace
			{
				scroller::thumb make_thumb(double width, double lbound, double ubound)
				{
					scroller::thumb t = { static_cast<real_t>(width), static_cast<real_t>(lbound), static_cast<real_t>(ubound) };
					return t;
				}

				struct eq
				{
					eq(double tolerance_ = 0.0001)
						: tolerance(tolerance_)
					{	}

					bool operator ()(double lhs, double rhs) const
					{
						if (double e = 2 * (lhs - rhs))
						{
							e /= lhs + rhs;
							return -tolerance <= e && e <= tolerance;
						}
						return true;
					}

					bool operator ()(scroller::thumb lhs, scroller::thumb rhs) const
					{
						return (*this)(lhs.width, rhs.width) && (*this)(lhs.lbound, rhs.lbound)
							&& (*this)(lhs.ubound, rhs.ubound);
					}

					bool operator ()(pair<double, double> lhs, pair<double, double> rhs) const
					{	return (*this)(lhs.first, rhs.first) && (*this)(lhs.second, rhs.second);	}

					double tolerance;
				};


				class capture_provider
				{
				public:
					void add_view(view &v)
					{
						_views[&v] = v.capture += [this, &v] (shared_ptr<void> &ch) {
							vector< pair<view *, bool> > &log_ = this->log;
							view &v2 = v;

							log_.push_back(make_pair(&v, true));
							ch.reset(new bool, [&log_, &v2] (bool *p) {
								delete p;
								log_.push_back(make_pair(&v2, false));
							});
						};
					}

				public:
					vector< pair<view *, bool> > log;

				private:
					map<view *, slot_connection> _views;
				};
			}

			begin_test_suite( AScrollerTests )
				typedef vector< pair<bool, agge::rect_i> > invalidations_log;

				wpl::ui::view::positioned_native_views dummy_nviews;

				static void invalidate_cb(invalidations_log *log, const agge::rect_i *w)
				{	log->push_back(w ? make_pair(true, *w) : make_pair(false, agge::rect_i()));	}

				function<void (const agge::rect_i *window)> log_invalidates(invalidations_log &log)
				{	return bind(&invalidate_cb, &log, _1);	}


				test( ModellessScrollerDoesNothingOnVisualEvents )
				{
					// INIT / ACT
					scroller s(scroller::horizontal);
					slot_connection iconn = s.invalidate += [] (const agge::rect_i *) { assert_is_true(false); };
					gcontext::surface_type surface(1000, 1000, 0);
					gcontext::renderer_type ren(1);
					gcontext::rasterizer_ptr ras(new gcontext::rasterizer_type);
					gcontext ctx(surface, ren, make_rect(103, 71, 130, 110));

					// ACT / ASSERT
					s.draw(ctx, ras);

					// ASSERT
					assert_equal(0, ras->height()); // a way to check emptiness

					// ACT / ASSERT (ok to call with zero dimensions)
					s.resize(100, 0, dummy_nviews);
					s.resize(0, 100, dummy_nviews);
					s.resize(0, 0, dummy_nviews);
				}


				test( ModellessScrollerDoesNothingOnMouseEvents )
				{
					// INIT / ACT
					scroller s(scroller::horizontal);
					slot_connection c = s.capture += [] (shared_ptr<void> &) { assert_is_true(false); };

					s.resize(100, 10, dummy_nviews);

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
					scroller sh(scroller::horizontal), sv(scroller::vertical);
					gcontext::surface_type surface(1000, 1000, 0);
					gcontext::renderer_type ren(1);
					gcontext::rasterizer_ptr ras(new gcontext::rasterizer_type);
					gcontext ctx(surface, ren, make_rect(103, 71, 130, 110));

					m->range = make_pair(0, 100);

					sh.resize(100, 20, dummy_nviews);
					sh.set_model(m);
					sv.resize(20, 100, dummy_nviews);
					sv.set_model(m);

					slot_connection iconnh = sh.invalidate += [] (const agge::rect_i *) { assert_is_true(false); };
					slot_connection iconnv = sv.invalidate += [] (const agge::rect_i *) { assert_is_true(false); };

					// ACT / ASSERT
					sh.resize(0, 10, dummy_nviews);
					sv.resize(10, 0, dummy_nviews);
					sh.draw(ctx, ras);
					sv.draw(ctx, ras);

					// ASSERT
					assert_equal(0, ras->height());

					// ACT / ASSERT
					sh.resize(100, 0, dummy_nviews);
					sv.resize(0, 100, dummy_nviews);
					sh.draw(ctx, ras);
					sv.draw(ctx, ras);

					// ASSERT
					assert_equal(0, ras->height());
				}


				test( ScrollerIsInvalidatedOnResizeToValidSize )
				{
					// INIT
					invalidations_log ilogh, ilogv;
					scroller sh(scroller::horizontal), sv(scroller::vertical);
					slot_connection c1 = sh.invalidate += log_invalidates(ilogh);
					slot_connection c2 = sv.invalidate += log_invalidates(ilogv);

					// ACT
					sh.resize(100, 20, dummy_nviews);
					sv.resize(20, 100, dummy_nviews);

					// ASSERT
					assert_equal(1u, ilogh.size());
					assert_is_false(ilogh[0].first);
					assert_equal(1u, ilogv.size());
					assert_is_false(ilogv[0].first);
				}


				test( ScrollerIsInvalidatedOnModelSetting )
				{
					// INIT
					invalidations_log ilogh, ilogv;
					shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
					scroller sh(scroller::horizontal), sv(scroller::vertical);
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


				test( ThumbCoordinatesAreCalculatedFromScrollerExtentAndModelRangeAndModelWindowNoLimitApplied )
				{
					// INIT
					shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
					scroller sh(scroller::horizontal), sv(scroller::vertical);

					m->range = make_pair(10.1, 100.7);
					m->window = make_pair(50.3, 14.5);
					sh.resize(94, 10, dummy_nviews);
					sv.resize(13, 194, dummy_nviews);

					sh.set_model(m);
					sv.set_model(m);

					// ACT / ASSERT
					assert_equal_pred(make_thumb(7, 37.53, 51.06), sh.get_thumb(), eq(0.0005));
					assert_equal_pred(make_thumb(9.1, 77.45, 105.4), sv.get_thumb(), eq(0.0005));

					// INIT
					m->window = make_pair(10.1, 25);

					// ACT / ASSERT
					assert_equal_pred(make_thumb(7, 0, 23.34), sh.get_thumb(), eq(0.0005));
					assert_equal_pred(make_thumb(9.1, 0, 48.16), sv.get_thumb(), eq(0.0005));

					// INIT
					m->range = make_pair(-100, 200);

					// ACT / ASSERT
					assert_equal_pred(make_thumb(7, 51.75, 63.5), sh.get_thumb(), eq(0.0005));
					assert_equal_pred(make_thumb(9.1, 106.8, 131), sv.get_thumb(), eq(0.0005));
				}


				test( PageScrollAttemptIsMadeWhenClickingOutsideThumb )
				{
					// INIT
					invalidations_log ilog;
					vector< pair<double, double> > log;
					shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
					scroller sh(scroller::horizontal), sv(scroller::vertical);

					m->range = make_pair(0, 500);
					m->window = make_pair(50, 27);
					m->on_scrolling = [] (bool) { assert_is_true(false); };
					m->on_scroll = [&] (double m, double w) { log.push_back(make_pair(m, w)); };

					sh.resize(100, 10, dummy_nviews);
					sh.set_model(m);
					sv.resize(1, 250, dummy_nviews);
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
					assert_equal_pred(reference1, log, eq());

					// INIT
					log.clear();

					// ACT
					sh.mouse_down(mouse_input::left, 0, 9, 0); // one page less

					// ASSERT
					assert_equal_pred(reference1, log, eq());

					// INIT
					log.clear();

					// ACT
					sh.mouse_down(mouse_input::left, 0, 16, 0); // one page more

					// ASSERT
					pair<double, double> reference2[] = { make_pair(77, 27), };

					assert_equal_pred(reference2, log, eq());

					// INIT
					log.clear();

					// ACT
					sh.mouse_down(mouse_input::left, 0, 99, 200 /*ignored*/); // one page more

					// ASSERT
					assert_equal_pred(reference2, log, eq());

					// INIT
					log.clear();

					// ACT
					sv.mouse_down(mouse_input::left, 0, -100 /*ignored*/, 0); // one page less

					// ASSERT
					assert_is_empty(ilog);
					assert_equal_pred(reference1, log, eq());

					// INIT
					log.clear();

					// ACT
					sv.mouse_down(mouse_input::left, 0, 0, 24); // one page less

					// ASSERT
					assert_equal_pred(reference1, log, eq());

					// INIT
					log.clear();

					// ACT
					sv.mouse_down(mouse_input::left, 0, 0, 39); // one page more

					// ASSERT
					assert_equal_pred(reference2, log, eq());

					// INIT
					log.clear();

					// ACT
					sv.mouse_down(mouse_input::left, 0, 100 /*ignored*/, 249); // one page more

					// ASSERT
					assert_equal_pred(reference2, log, eq());

					// INIT
					m->range = make_pair(21.5, 200);
					m->window = make_pair(51.5, 20);
					log.clear();

					// ACT
					sh.mouse_down(mouse_input::left, 0, 14, -10); // one page less

					// ASSERT
					pair<double, double> reference3[] = { make_pair(31.5, 20), };

					assert_equal_pred(reference3, log, eq());

					// INIT
					log.clear();

					// ACT
					sh.mouse_down(mouse_input::left, 0, 25, 10); // one page more

					// ASSERT
					pair<double, double> reference4[] = { make_pair(71.5, 20), };

					assert_equal_pred(reference4, log, eq());
				}


				test( PageScrollAttemptIsLimitedToTheRangeBoundaries )
				{
					// INIT
					vector< pair<double, double> > log;
					shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
					scroller s(scroller::horizontal);

					m->range = make_pair(110.7, 150.1);
					m->window = make_pair(125, 20);
					m->on_scroll = [&] (double m, double w) { log.push_back(make_pair(m, w)); };

					s.resize(100, 10, dummy_nviews);
					s.set_model(m);

					slot_connection c = s.capture += [] (shared_ptr<void> &/*handle*/) { assert_is_true(false); };

					// ACT
					s.mouse_down(mouse_input::left, 0, 0, 0); // one page less

					// ASSERT
					pair<double, double> reference1[] = { make_pair(110.7, 20), };

					assert_equal_pred(reference1, log, eq());

					// INIT
					log.clear();
					m->window = make_pair(230, 21);

					// ACT
					s.mouse_down(mouse_input::left, 0, 99, 0); // one page more

					// ASSERT
					pair<double, double> reference2[] = { make_pair(239.8, 21), };

					assert_equal_pred(reference2, log, eq());
				}


				test( ModelInvalidationLeadsToControlInvalidation )
				{
					shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
					scroller s(scroller::horizontal);
					invalidations_log ilog;

					s.resize(100, 10, dummy_nviews);
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


				test( LButtonCycleWithinThumbCapturesReleasesMouse )
				{
					// INIT
					capture_provider cp;
					invalidations_log ilog;
					shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
					scroller sh(scroller::horizontal), sv(scroller::vertical);
					bool scrolling = false;

					m->range = make_pair(0, 500);
					m->window = make_pair(50, 40);
					m->on_scrolling = [&] (bool begins) {
						scrolling = begins;
						assert_is_empty(cp.log); // must get called before the mouse is captured.
					};
					m->on_scroll = [&] (double, double) { assert_is_true(false); };

					sh.resize(100, 10, dummy_nviews);
					sh.set_model(m);
					sv.resize(30, 250, dummy_nviews);
					sv.set_model(m);

					cp.add_view(sh);
					cp.add_view(sv);

					// ACT
					sh.mouse_down(mouse_input::left, 0, 10, 5);

					// ASSERT
					pair<view *, bool> reference1[] = { make_pair(&sh, true), };

					assert_is_true(scrolling);
					assert_equal(reference1, cp.log);

					// INIT
					cp.log.clear();
					scrolling = false;

					// ACT
					sv.mouse_down(mouse_input::left, 0, 15, 25);

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
					scroller s(scroller::horizontal);

					m->range = make_pair(10, 101);
					m->window = make_pair(50, 20);
					m->on_scroll = [&] (double, double) { assert_is_true(false); };

					s.resize(100, 10, dummy_nviews);
					s.set_model(m);

					// ACT / ASSERT
					s.mouse_move(mouse_input::left, 49, 100);
					s.mouse_move(mouse_input::left, 51, 100);
				}


				test( MovingMouseWhileCapturedMovesTheWindow )
				{
					// INIT
					capture_provider cp;
					vector< pair<double, double> > log;
					shared_ptr<mocks::scroll_model> m(new mocks::scroll_model);
					scroller sh(scroller::horizontal), sv(scroller::vertical);

					m->range = make_pair(10, 101);
					m->window = make_pair(50, 20);
					m->on_scroll = [&] (double m, double w) { log.push_back(make_pair(m, w)); };

					sh.resize(100, 10, dummy_nviews);
					cp.add_view(sh);
					sh.set_model(m);
					sv.resize(10, 300, dummy_nviews);
					cp.add_view(sv);
					sv.set_model(m);

					sh.mouse_down(mouse_input::left, 0, 50, 5);
					sv.mouse_down(mouse_input::left, 0, 5, 150);

					assert_equal(2u, cp.log.size());

					// ACT
					m->window = make_pair(52, 21); // try to spoil the results - make sure the initial window is stored.
					sh.mouse_move(mouse_input::left, 49, 100);

					// ASSERT
					pair<double, double> reference1[] = { make_pair(50 - 1.01, 20), };

					assert_equal_pred(reference1, log, eq());

					// ACT
					m->window = make_pair(12, 1);
					sh.mouse_move(mouse_input::left, 40, 100);

					// ASSERT
					pair<double, double> reference2[] = {
						make_pair(50 - 1.01, 20),
						make_pair(50 - 10.1, 20),
					};

					assert_equal_pred(reference2, log, eq());

					// ACT
					sh.mouse_move(mouse_input::left, 61, -10);

					// ASSERT
					pair<double, double> reference3[] = {
						make_pair(50 - 1.01, 20),
						make_pair(50 - 10.1, 20),
						make_pair(50 + 1.01 * 11, 20),
					};

					assert_equal_pred(reference3, log, eq());

					// ACT
					sv.mouse_move(mouse_input::left, 5, 152);

					// ASSERT
					pair<double, double> reference4[] = {
						make_pair(50 - 1.01, 20),
						make_pair(50 - 10.1, 20),
						make_pair(50 + 1.01 * 11, 20),
						make_pair(50 + 2.0 / 300.0 * 101.0, 20),
					};

					assert_equal_pred(reference4, log, eq());
				}

			end_test_suite
		}
	}
}
