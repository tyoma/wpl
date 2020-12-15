#include <wpl/animated_models.h>

#include <tests/common/mock-queue.h>
#include <tests/common/MockupsScroller.h>
#include <tests/common/predicates.h>

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		begin_test_suite( AnimatedScrollModelTests )
			clock clock_;
			queue queue_;

			timestamp time;
			mocks::queue_container queued;

			init( Init )
			{
				clock_ = [this] { return this->time; };
				queue_ = mocks::create_queue(queued);
			}


			test( ModelPassesThroughUnderlyingValues )
			{
				// INIT
				shared_ptr<mocks::scroll_model> u(new mocks::scroll_model);

				u->range = make_pair(17.1, 101.5);
				u->window = make_pair(20.2, 10.7);
				u->increment = 3.0;

				// INIT/ ACT
				animated_scroll_model m(u, clock_, queue_, &no_animation);

				// ACT / ASSERT
				assert_equal(make_pair(17.1, 101.5), m.get_range());
				assert_equal(make_pair(20.2, 10.7), m.get_window());
				assert_equal(3.0, m.get_increment());

				// INIT
				u->range = make_pair(11.2, 1001.5);
				u->window = make_pair(15.2, 11.1);
				u->increment = 7.2;

				// ACT / ASSERT
				assert_equal(make_pair(11.2, 1001.5), m.get_range());
				assert_equal(make_pair(15.2, 11.1), m.get_window());
				assert_equal(7.2, m.get_increment());

				// INIT
				u->window = make_pair(11.2, 11.1);

				// ACT / ASSERT
				assert_equal(make_pair(11.2, 11.1), m.get_window());
			}


			test( InvalidationsArePassedOnDownstream )
			{
				// INIT
				auto invalidate = 0;
				shared_ptr<mocks::scroll_model> u(new mocks::scroll_model);
				animated_scroll_model m(u, clock_, queue_, &no_animation);
				auto conn = m.invalidate += [&] {	invalidate++;	};

				// ACT
				u->invalidate();

				// ASSERT
				assert_equal(1, invalidate);

				// ACT
				u->invalidate();
				u->invalidate();

				// ASSERT
				assert_equal(3, invalidate);
			}


			test( ScrollingInsideRangeIsPassedAsIsToUnderlying )
			{
				// INIT
				shared_ptr<mocks::scroll_model> u(new mocks::scroll_model);
				animated_scroll_model m(u, clock_, queue_, &no_animation);
				vector< pair<double, double> > log;

				u->range = make_pair(10.1, 101.5);
				u->window = make_pair(20.2, 10.7);
				u->increment = 3.0;
				u->on_scroll = [&] (double b, double w) { log.push_back(make_pair(b, w)); };

				// ACT
				m.scrolling(true);
				m.scroll_window(15.6, 10.7);
				m.scroll_window(25.6, 10.7);
				m.scrolling(false);

				// ASSERT
				pair<double, double> reference1[] = {
					make_pair(15.6, 10.7), make_pair(25.6, 10.7),
				};

				assert_equal(reference1, log);

				// INIT
				log.clear();

				// ACT
				m.scrolling(true);
				m.scroll_window(50.1, 10.7);
				m.scrolling(false);

				// ASSERT
				pair<double, double> reference2[] = {
					make_pair(50.1, 10.7),
				};

				assert_equal(reference2, log);
			}


			test( WindowSqueezesWhenUnderlyingProvidesOutOfRangeValuesLower )
			{
				// INIT
				shared_ptr<mocks::scroll_model> u(new mocks::scroll_model);
				animated_scroll_model m(u, clock_, queue_, &no_animation);
				vector< pair<double, double> > log;

				u->range = make_pair(0, 100);
				u->window = make_pair(-1.0, 10.0);

				// ACT / ASSERT
				assert_equal_pred(make_pair(0.0, 9.0), m.get_window(), eq());

				// INIT
				u->window = make_pair(-8.0, 10.0);

				// ACT / ASSERT
				assert_equal_pred(make_pair(0.0, 2.0), m.get_window(), eq());

				// INIT
				u->window = make_pair(-80.0, 10.0);

				// ACT / ASSERT
				assert_equal_pred(make_pair(0.0, 0.1 /*0.01 * window*/), m.get_window(), eq());

				// INIT
				u->range = make_pair(-70.5, 1000);
				u->window = make_pair(-80.0, 55.0);

				// ACT / ASSERT
				assert_equal_pred(make_pair(-70.5, 45.5), m.get_window(), eq());

				// INIT
				u->window = make_pair(-200.0, 55.0);

				// ACT / ASSERT
				assert_equal_pred(make_pair(-70.5, 0.55), m.get_window(), eq());
			}


			test( WindowSqueezesWhenUnderlyingProvidesOutOfRangeValuesUpper )
			{
				// INIT
				shared_ptr<mocks::scroll_model> u(new mocks::scroll_model);
				animated_scroll_model m(u, clock_, queue_, &no_animation);
				vector< pair<double, double> > log;

				u->range = make_pair(0, 100);
				u->window = make_pair(95.2, 10.0);

				// ACT / ASSERT
				assert_equal_pred(make_pair(95.2, 4.8), m.get_window(), eq());

				// INIT
				u->window = make_pair(98.0, 10.0);

				// ACT / ASSERT
				assert_equal_pred(make_pair(98.0, 2.0), m.get_window(), eq());

				// INIT
				u->window = make_pair(110, 10.0);

				// ACT / ASSERT
				assert_equal_pred(make_pair(99.9, 0.1 /*0.01 * window*/), m.get_window(), eq());

				// INIT
				u->range = make_pair(-70.5, 1000);
				u->window = make_pair(910.0, 55.0);

				// ACT / ASSERT
				assert_equal_pred(make_pair(910.0, 19.5), m.get_window(), eq());

				// INIT
				u->window = make_pair(905.0, 55.0);

				// ACT / ASSERT
				assert_equal_pred(make_pair(905.0, 24.5), m.get_window(), eq());

				// INIT
				u->window = make_pair(1983.2, 55.0);

				// ACT / ASSERT
				assert_equal_pred(make_pair(929.0, 0.55), m.get_window(), eq());
			}


			test( ScrollingOutsideTheRangeIsSlowingDownLower )
			{
				// INIT
				shared_ptr<mocks::scroll_model> u(new mocks::scroll_model);
				animated_scroll_model m(u, clock_, queue_, &no_animation);
				vector< pair<double, double> > log;

				u->range = make_pair(0, 100);
				u->window = make_pair(10, 11);
				u->on_scroll = [&] (double b, double w) { log.push_back(make_pair(b, w)); };

				// ACT
				m.scrolling(true);
				m.scroll_window(0, 0 /*irrelevant*/);
				m.scroll_window(-1000000, 123 /*irrelevant*/);

				// ASSERT
				pair<double, double> reference1[] = {
					make_pair(0, 11), make_pair(-11, 11),
				};

				assert_equal_pred(reference1, log, eq());

				// INIT
				log.clear();
				u->window = make_pair(-10, 10);

				// ACT
				m.scroll_window(-10, 0 /*irrelevant*/);

				// ASSERT
				pair<double, double> reference2[] = {
					make_pair(-5, 10),
				};

				assert_equal_pred(reference2, log, eq());

				// INIT
				log.clear();
				u->window = make_pair(0, 5);

				// ACT
				m.scroll_window(-15, 0 /*irrelevant*/);

				// ASSERT
				pair<double, double> reference3[] = {
					make_pair(-3.75, 5),
				};

				assert_equal_pred(reference3, log, eq());
			}


			test( ScrollingOutsideTheRangeIsSlowingDownUpper )
			{
				// INIT
				shared_ptr<mocks::scroll_model> u(new mocks::scroll_model);
				animated_scroll_model m(u, clock_, queue_, &no_animation);
				vector< pair<double, double> > log;

				u->range = make_pair(0, 100);
				u->window = make_pair(10, 11);
				u->on_scroll = [&] (double b, double w) { log.push_back(make_pair(b, w)); };

				// ACT
				m.scrolling(true);
				m.scroll_window(89, 0 /*irrelevant*/);
				m.scroll_window(1000000, 123 /*irrelevant*/);

				// ASSERT
				pair<double, double> reference1[] = {
					make_pair(89, 11), make_pair(100, 11),
				};

				assert_equal_pred(reference1, log, eq());

				// INIT
				log.clear();
				u->window = make_pair(10, 10);

				// ACT
				m.scroll_window(100, 0 /*irrelevant*/);

				// ASSERT
				pair<double, double> reference2[] = {
					make_pair(95, 10),
				};

				assert_equal_pred(reference2, log, eq());

				// INIT
				log.clear();
				u->window = make_pair(0, 5);

				// ACT
				m.scroll_window(110, 0 /*irrelevant*/);

				// ASSERT
				pair<double, double> reference3[] = {
					make_pair(98.75, 5),
				};

				assert_equal_pred(reference3, log, eq());
			}


			test( NoAnimationTaskIsScheduledOnScrollEndsWithinTheRange )
			{
				// INIT
				shared_ptr<mocks::scroll_model> u(new mocks::scroll_model);
				animated_scroll_model m(u, clock_, queue_, &no_animation);
				vector<int> events;

				u->range = make_pair(0, 100);
				u->window = make_pair(10, 11);
				u->on_scrolling = [&] (bool begins) { events.push_back(begins ? 1 : 3); };
				u->on_scroll = [&] (...) { events.push_back(2); };

				// ACT
				m.scrolling(true);
				m.scroll_window(0, 11);
				m.scroll_window(89, 11);
				m.scroll_window(50, 11);
				m.scrolling(false);

				// ASSERT
				int reference[] = { 1, 2, 2, 2, 3, };

				assert_equal(reference, events);
				assert_is_empty(queued);

				// INIT
				u->range = make_pair(1000, 50);
				u->window = make_pair(1040, 5);
				events.clear();

				// ACT
				m.scrolling(true);
				m.scroll_window(1000, 5);
				m.scroll_window(1045, 5);
				m.scroll_window(1010, 5);
				m.scrolling(false);

				// ASSERT
				assert_equal(reference, events);
				assert_is_empty(queued);
			}


			test( AnimationTaskIsScheduledIfScrollEndsOutsideTheRange )
			{
				// INIT
				shared_ptr<mocks::scroll_model> u(new mocks::scroll_model);
				animated_scroll_model m1(u, clock_, queue_, &no_animation);
				animated_scroll_model m2(u, clock_, queue_, &no_animation);
				vector<int> events;

				u->range = make_pair(0, 100);
				u->window = make_pair(10, 10);
				u->on_scrolling = [&] (bool begins) { events.push_back(begins ? 1 : 3); };
				u->on_scroll = [&] (...) { events.push_back(2); };

				// ACT
				m1.scrolling(true);
				m1.scroll_window(-10, 10);
				m1.scrolling(false);

				// ASSERT
				int reference[] = { 1, 2, };

				assert_equal(reference, events);
				assert_equal(1u, queued.size());
				assert_equal(10, queued.front().defer_by);

				// INIT
				events.clear();
				queued.pop();

				// ACT
				m2.scrolling(true);
				m2.scroll_window(-10, 10);
				m2.scrolling(false);

				// ASSERT
				assert_equal(reference, events);
				assert_equal(1u, queued.size());
				assert_equal(10, queued.front().defer_by);
			}


			test( AnimationTaskScrollsRangeAccordinglyToAnimationFunctionLower )
			{
				// INIT
				shared_ptr<mocks::scroll_model> u(new mocks::scroll_model);
				double progress;
				auto result = true;
				auto invalidate = 0;
				animated_scroll_model m(u, clock_, queue_, [&] (double &p, double) -> bool {
					p = progress;
					return result;
				});
				auto conn = m.invalidate += [&] {	invalidate++;	};
				vector< pair<double, double> > log;
				vector<int> events;

				u->range = make_pair(10, 104);
				u->window = make_pair(10, 7);
				u->on_scrolling = [&] (bool begins) { events.push_back(begins ? 1 : 3); };
				u->on_scroll = [&] (double m, double w) {
					events.push_back(2);
					log.push_back(make_pair(m, w));
				};

				m.scrolling(true);
				m.scroll_window(150, 7);
				m.scrolling(false);

				// ACT
				progress = 0.1;
				queued.front().task();
				queued.pop();

				// ASSERT
				int reference1_events[] = { 1, 2, 2, };

				assert_equal(reference1_events, events);
				assert_equal_pred(make_pair(log[0].first * 0.9 + 107 * 0.1, 7), log[1], eq());
				assert_equal(1u, queued.size());
				assert_equal(10, queued.front().defer_by);
				assert_equal(1, invalidate);

				// ACT
				time = 122991;
				progress = 0.81;
				queued.front().task();
				queued.pop();

				// ASSERT
				int reference2_events[] = { 1, 2, 2, 2, };

				assert_equal(reference2_events, events);
				assert_equal_pred(make_pair(log[0].first * 0.19 + 107 * 0.81, 7), log[2], eq());
				assert_equal(1u, queued.size());
				assert_equal(10, queued.front().defer_by);
				assert_equal(2, invalidate);

				// INIT
				u->range = make_pair(3, 100);

				// ACT
				result = false;
				time = 123123;
				progress = 1.0;
				queued.front().task();
				queued.pop();

				// ASSERT
				int reference3_events[] = { 1, 2, 2, 2, 2, 3, };

				assert_equal(reference3_events, events);
				assert_equal_pred(make_pair(96, 7), log[3], eq());
				assert_is_empty(queued);
			}
		end_test_suite
	}
}
