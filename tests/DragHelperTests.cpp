#include <wpl/drag_helper.h>

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		begin_test_suite( DragHelperTests )
			test( NewHelperIsNotActive )
			{
				// INIT
				drag_helper dh;

				// ACT / ASSERT
				assert_is_false(dh.mouse_move(10, 11));
			}


			test( StartingDragObtainsCapture )
			{
				// INIT
				auto captured = 0;
				const auto on_capture = [&] (shared_ptr<void> &h) {
					captured++;
					h.reset(new int, [&] (int *p) {
						captured--;
						delete p;
					});
				};
				drag_helper dh;

				// ACT
				dh.start([] (int, int) {}, on_capture, mouse_input::left, 0, 0);

				// ACT / ASSERT
				assert_is_true(dh.mouse_move(10, 11));

				// ASSERT
				assert_equal(1, captured);
			}


			test( CancellingDragReleasesCapture )
			{
				// INIT
				auto captured = 0;
				const auto on_capture = [&] (shared_ptr<void> &h) {
					captured++;
					h.reset(new int, [&] (int *p) {
						captured--;
						delete p;
					});
				};
				drag_helper dh;

				dh.start([] (int, int) {}, on_capture, mouse_input::left, 0, 0);

				// ACT
				dh.cancel();

				// ACT / ASSERT
				assert_is_false(dh.mouse_move(10, 11));

				// ASSERT
				assert_equal(0, captured);
			}


			test( EndingDragReleasesCapture )
			{
				// INIT
				auto captured = 0;
				const auto on_capture = [&] (shared_ptr<void> &h) {
					captured++;
					h.reset(new int, [&] (int *p) {
						captured--;
						delete p;
					});
				};
				drag_helper dh;

				dh.start([] (int, int) {}, on_capture, mouse_input::left, 0, 0);

				// ACT / ASSERT
				assert_is_true(dh.mouse_up(mouse_input::left));
				assert_is_false(dh.mouse_move(10, 11));

				// ASSERT
				assert_equal(0, captured);

				// ACT / ASSERT
				assert_is_false(dh.mouse_up(mouse_input::left));
			}


			test( UnexpectedButtonDoesNotEndDrag )
			{
				// INIT
				auto captured = 0;
				const auto on_capture = [&] (shared_ptr<void> &h) {
					captured++;
					h.reset(new int, [&] (int *p) {
						captured--;
						delete p;
					});
				};
				drag_helper dh;

				dh.start([] (int, int) {}, on_capture, mouse_input::left, 0, 0);

				// ACT
				dh.mouse_up(mouse_input::right);

				// ACT / ASSERT
				assert_is_true(dh.mouse_move(10, 11));

				// ASSERT
				assert_equal(1, captured);

				// ACT
				dh.mouse_up(mouse_input::middle);

				// ASSERT
				assert_equal(1, captured);

				// INIT
				dh.mouse_up(mouse_input::left);
				dh.start([] (int, int) {}, on_capture, mouse_input::right, 0, 0);

				// ACT
				dh.mouse_up(mouse_input::left);

				// ASSERT
				assert_equal(1, captured);
			}


			test( DraggingInvokesOnDragWithDeltas )
			{
				// INIT
				vector< pair<int, int> > log;
				const auto on_drag = [&] (int dx, int dy) {
					log.push_back(make_pair(dx, dy));
				};
				drag_helper dh;

				dh.start(on_drag, [] (shared_ptr<void> &) { }, mouse_input::left, 11, 19);

				// ACT / ASSERT
				assert_is_true(dh.mouse_move(100, 100));

				// ASSERT
				pair<int, int> reference1[] = {	make_pair(89, 81),	};

				assert_equal(reference1, log);

				// ACT
				dh.mouse_move(10, 10);

				// ASSERT
				pair<int, int> reference2[] = {	make_pair(89, 81), make_pair(-1, -9),	};

				assert_equal(reference2, log);

				// INIT
				dh.start(on_drag, [] (shared_ptr<void> &) { }, mouse_input::left, 100, 100);
				log.clear();

				// ACT
				dh.mouse_move(10, 10);

				// ASSERT
				pair<int, int> reference3[] = {	make_pair(-90, -90),	};

				assert_equal(reference3, log);
			}


			test( OnDragWithZeroesIsCalledOnCancel )
			{
				// INIT
				vector< pair<int, int> > log;
				const auto on_drag = [&] (int dx, int dy) {
					log.push_back(make_pair(dx, dy));
				};
				drag_helper dh;

				dh.start(on_drag, [] (shared_ptr<void> &) { }, mouse_input::left, 11, 19);
				dh.mouse_move(100, 100);

				// ACT
				dh.cancel();

				// ASSERT
				pair<int, int> reference[] = {	make_pair(89, 81), make_pair(0, 0),	};

				assert_equal(reference, log);

				// ACT
				dh.cancel();

				// ASSERT
				assert_equal(reference, log);
			}

		end_test_suite;
	}
}
