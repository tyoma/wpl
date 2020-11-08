#include <wpl/animated_models.h>

#include "MockupsScroller.h"
#include "predicates.h"

#include <tq/mocks/queue.h>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		begin_test_suite( AnimatedScrollModelTests )
			shared_ptr<tq::mocks::queue> queue;

			init( Init )
			{
				queue.reset(new tq::mocks::queue);
			}

			test( ModelPassesThroughUnderlyingValues )
			{
				// INIT 
				shared_ptr<mocks::scroll_model> u(new mocks::scroll_model);

				u->range = make_pair(17.1, 101.5);
				u->window = make_pair(20.2, 10.7);
				u->increment = 3.0;

				// INIT / ACT
				animated_scroll_model m(u, queue);

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


			test( ScrollingInsideRangeIsPassedAsIsToUnderlying )
			{
				// INIT 
				shared_ptr<mocks::scroll_model> u(new mocks::scroll_model);
				animated_scroll_model m(u, queue);
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
				assert_equal(0u, queue->get_task_count());
			}


			test( RangeSqueezesWhenUnderlyingProvidesOutOfRangeValuesLower )
			{
				// INIT 
				shared_ptr<mocks::scroll_model> u(new mocks::scroll_model);
				animated_scroll_model m(u, queue);
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


			test( RangeSqueezesWhenUnderlyingProvidesOutOfRangeValuesUpper )
			{
				// INIT 
				shared_ptr<mocks::scroll_model> u(new mocks::scroll_model);
				animated_scroll_model m(u, queue);
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
				animated_scroll_model m(u, queue);
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
				animated_scroll_model m(u, queue);
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
		end_test_suite
	}
}
