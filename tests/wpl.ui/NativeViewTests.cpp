#include <src/wpl.ui/win32/native_view.h>

#include "TestHelpers.h"

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace mocks
			{
				class windowed_view : public win32::native_view<visual>
				{
				public:
					vector< pair<HWND /*parent*/, HWND /*created*/> > log;

				private:
					virtual LRESULT on_message(UINT message, WPARAM wparam, LPARAM lparam,
						const window::original_handler_t &original)
					{	return original(message, wparam, lparam);	}

					virtual HWND materialize(HWND hparent)
					{
						HWND hwnd = ::CreateWindow(L"static", 0, WS_CHILD, 0, 0, 1, 1, hparent, 0, 0, 0);

						log.push_back(make_pair(hparent, hwnd));
						return hwnd;
					}
				};
			}

			begin_test_suite( NativeViewHelperTests )
				WindowManager wmanager;
				HWND parent;
				window_tracker tracker;

				init( CreateParent )
				{
					parent = wmanager.create_visible_window();
					tracker.checkpoint();
					tracker.created.clear();
				}


				test( NativeViewHelperDoesNotCreateAnythingOnConstruction )
				{
					// INIT / ACT
					mocks::windowed_view wv;

					// ASSERT
					tracker.checkpoint();

					assert_is_empty(tracker.created);
				}


				test( NativeViewHelperRequestsWindowCreationOnGetWindow )
				{
					// INIT
					mocks::windowed_view wv;
					native_view &nv = wv;

					// ACT
					HWND hwnd = nv.get_window(parent);

					// ASSERT
					pair<HWND, HWND> reference[] = { make_pair(parent, hwnd) };

					assert_equal(reference, wv.log);
				}


				test( WindowIsMaterializedOnlyOnceForTheSameParent )
				{
					// INIT
					mocks::windowed_view wv;
					native_view &nv = wv;

					HWND hwnd1 = nv.get_window(parent);

					// ACT
					HWND hwnd2 = nv.get_window(parent);

					// ASSERT
					assert_equal(1u, wv.log.size());
					assert_equal(hwnd1, hwnd2);
				}


				test( WindowIsRecreatedForANewParent )
				{
					// INIT
					mocks::windowed_view wv;
					native_view &nv = wv;
					HWND parent2 = wmanager.create_visible_window();

					HWND hwnd1 = nv.get_window(parent);

					tracker.checkpoint();

					// ACT
					HWND hwnd2 = nv.get_window(parent2);

					// ASSERT
					HWND destroyed[] = { hwnd1, };

					tracker.checkpoint();
					assert_equal(destroyed, tracker.destroyed);
					assert_not_equal(hwnd1, hwnd2);
				}


				test( OwnedWindowIsDestroyedOnViewDestruction )
				{
					// INIT
					shared_ptr<mocks::windowed_view> wv(new mocks::windowed_view);
					native_view &nv = *wv;

					HWND hwnd = nv.get_window(parent);

					tracker.checkpoint();

					// ACT
					wv.reset();

					// ASSERT
					HWND destroyed[] = { hwnd, };

					tracker.checkpoint();
					assert_equal(destroyed, tracker.destroyed);
				}


			end_test_suite
		}
	}
}
