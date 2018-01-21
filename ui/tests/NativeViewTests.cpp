#include <wpl/ui/win32/native_view.h>

#include <wpl/ui/geometry.h>

#include "Mockups.h"
#include "TestHelpers.h"

#include <ut/assert.h>
#include <ut/test.h>
#include <windows.h>

using namespace std;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			begin_test_suite( NativeViewTests )

				WindowManager windowManager;

				init( Init )
				{
					windowManager.Init();
				}

				teardown( Cleanup )
				{
					windowManager.Cleanup();
				}

				test( WindowIsValidOnConstruction )
				{
					// INIT
					shared_ptr<mocks::TestNativeWidget> w(new mocks::TestNativeWidget);

					// ACT / ASSERT
					assert_is_true(!!::IsWindow(w->hwnd()));
				}

				
				test( WindowIsDestroyedAtWidgetDeleted )
				{
					// INIT
					shared_ptr<mocks::TestNativeWidget> w(new mocks::TestNativeWidget);
					HWND hwnd = w->hwnd();

					// ACT
					w = shared_ptr<mocks::TestNativeWidget>();
					
					// ASSERT
					assert_is_false(!!::IsWindow(hwnd));
				}


				test( SettingParentOnNativeViewChangesTestNativeWidgetsParentWindow )
				{
					// INIT
					HWND hparent1 = windowManager.create_window(), hparent2 = windowManager.create_window();
					shared_ptr<mocks::TestNativeWidget> w(new mocks::TestNativeWidget);
					shared_ptr<view> v;

					// ACT
					v = w->create_view(native_root(hparent1));

					// ASSERT
					assert_equal(::GetParent(w->hwnd()), hparent1);

					// ACT
					v = w->create_view(native_root(hparent2));

					// ASSERT
					assert_equal(::GetParent(w->hwnd()), hparent2);
				}


				test( WidgetIsHeldByItsView )
				{
					// INIT
					shared_ptr<widget> w_strong(new mocks::TestNativeWidget);
					weak_ptr<widget> w_weak(w_strong);
					shared_ptr<view> v(w_strong->create_view(native_root(0)));

					// ACT
					w_strong = shared_ptr<widget>();

					// ASSERT
					assert_is_false(w_weak.expired());
				}


				test( WidgetIsDestroyedOnJunctionDestroy )
				{
					// INIT
					shared_ptr<widget> w_strong(new mocks::TestNativeWidget);
					weak_ptr<widget> w_weak(w_strong);
					shared_ptr<view> v(w_strong->create_view(native_root(0)));

					// ACT
					w_strong = shared_ptr<widget>();
					v = shared_ptr<view>();

					// ASSERT
					assert_is_true(w_weak.expired());
				}


				test( MoveNativeView )
				{
					// INIT
					HWND hparent = windowManager.create_window();
					shared_ptr<mocks::TestNativeWidget> w(new mocks::TestNativeWidget);
					shared_ptr<view> v(w->create_view(native_root(hparent)));
					RECT rc;

					// ACT
					v->move(1, 3, 10, 20);

					// ASSERT
					rc = get_window_rect(w->hwnd());

					assert_equal(1, rc.left);
					assert_equal(3, rc.top);
					assert_equal(11, rc.right);
					assert_equal(23, rc.bottom);

					// ACT
					v->move(7, 11, 13, 29);

					// ASSERT
					rc = get_window_rect(w->hwnd());

					assert_equal(7, rc.left);
					assert_equal(11, rc.top);
					assert_equal(20, rc.right);
					assert_equal(40, rc.bottom);
				}
			end_test_suite
		}
	}
}
