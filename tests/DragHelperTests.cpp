#include <wpl/drag_helper.h>

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		begin_test_suite( DragHelperTests )
			test( StartingDragObtainsCapture )
			{
				// INIT
				auto captured = 0;
				const auto on_capture = [&] (shared_ptr<void> &h, mouse_input &/*target*/) {
					captured++;
					h.reset(new int, [&] (int *p) {
						captured--;
						delete p;
					});
				};
				drag_helper dh;

				// ACT
				dh.start([] (int, int) {}, [] {	}, on_capture, mouse_input::left, 0, 0);

				// ASSERT
				assert_equal(1, captured);
			}


			test( CancellingDragReleasesCapture )
			{
				// INIT
				auto captured = 0;
				const auto on_capture = [&] (shared_ptr<void> &h, mouse_input &/*target*/) {
					captured++;
					h.reset(new int, [&] (int *p) {
						captured--;
						delete p;
					});
				};
				auto complete = make_shared<bool>(false);
				drag_helper dh;

				dh.start([] (int, int) {}, [complete] {	*complete = true;	}, on_capture, mouse_input::left, 0, 0);

				// ACT
				dh.cancel();

				// ASSERT
				assert_equal(0, captured);
				assert_is_false(*complete);
				assert_equal(1, complete.use_count());
			}


			test( EndingDragReleasesCapture )
			{
				// INIT
				auto captured = 0;
				mouse_input *target = nullptr;
				const auto on_capture = [&] (shared_ptr<void> &h, mouse_input &target_) {
					captured++;
					target = &target_;
					h.reset(new int, [&] (int *p) {
						captured--;
						delete p;
					});
				};
				auto complete = make_shared<bool>(false);
				drag_helper dh;

				// INIT / ACT
				dh.start([] (int, int) {}, [complete] {	*complete = true;	}, on_capture, mouse_input::left, 0, 0);

				// ASSERT
				assert_is_false(*complete);
				assert_is_true(complete.use_count() > 1);

				// ACT
				target->mouse_up(mouse_input::left, 0, 0, 0);

				// ASSERT
				assert_equal(0, captured);
				assert_is_true(*complete);
				assert_equal(1, complete.use_count());
			}


			test( UnexpectedButtonDoesNotEndDrag )
			{
				// INIT
				auto captured = 0;
				mouse_input *target = nullptr;
				const auto on_capture = [&] (shared_ptr<void> &h, mouse_input &target_) {
					captured++;
					target = &target_;
					h.reset(new int, [&] (int *p) {
						captured--;
						delete p;
					});
				};
				drag_helper dh;

				dh.start([] (int, int) {}, [] {	}, on_capture, mouse_input::right, 0, 0);

				// ACT
				target->mouse_up(mouse_input::left, 0, 0, 0);

				// ASSERT
				assert_equal(1, captured);

				// ACT
				target->mouse_up(mouse_input::middle, 0, 0, 0);

				// ASSERT
				assert_equal(1, captured);

				// INIT
				target->mouse_up(mouse_input::right, 0, 0, 0);
				dh.start([] (int, int) {}, [] {	}, on_capture, mouse_input::middle, 0, 0);

				// ACT
				target->mouse_up(mouse_input::left, 0, 0, 0);

				// ASSERT
				assert_equal(1, captured);
			}


			test( DraggingInvokesOnDragWithDeltas )
			{
				// INIT
				vector< pair<int, int> > log;
				mouse_input *target = nullptr;
				const auto on_drag = [&] (int dx, int dy) {
					log.push_back(make_pair(dx, dy));
				};
				drag_helper dh;

				dh.start(on_drag, [] {	}, [&] (shared_ptr<void> &, mouse_input &target_) {
					target = &target_;
				}, mouse_input::left, 11, 19);

				// ACT / ASSERT
				target->mouse_move(mouse_input::left, 100, 100);

				// ASSERT
				pair<int, int> reference1[] = {	make_pair(89, 81),	};

				assert_equal(reference1, log);

				// ACT
				target->mouse_move(mouse_input::right | mouse_input::middle, 10, 10);

				// ASSERT
				pair<int, int> reference2[] = {	make_pair(89, 81), make_pair(-1, -9),	};

				assert_equal(reference2, log);

				// INIT
				dh.start(on_drag, [] {	}, [&] (shared_ptr<void> &, mouse_input &target_) {
					target = &target_;
				}, mouse_input::left, 100, 100);
				log.clear();

				// ACT
				target->mouse_move(mouse_input::right, 10, 10);

				// ASSERT
				pair<int, int> reference3[] = {	make_pair(-90, -90),	};

				assert_equal(reference3, log);
			}


			test( OnDragWithZeroesIsCalledOnCancel )
			{
				// INIT
				vector< pair<int, int> > log;
				mouse_input *target = nullptr;
				const auto on_drag = [&] (int dx, int dy) {
					log.push_back(make_pair(dx, dy));
				};
				drag_helper dh;

				dh.start(on_drag, [] {	}, [&] (shared_ptr<void> &, mouse_input &target_) {
					target = &target_;
				}, mouse_input::left, 11, 19);
				target->mouse_move(0, 100, 100);

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
