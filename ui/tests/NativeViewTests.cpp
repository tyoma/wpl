#include <wpl/ui/win32/native_view.h>

#include "TestWidgets.h"

#include <wpl/ui/geometry.h>

#include "TestHelpers.h"

#include <windows.h>

namespace std
{
   using tr1::weak_ptr;
}

using namespace std;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			[TestClass]
			public ref class NativeViewTests : ut::WindowTestsBase
			{
			public:
				[TestMethod]
				void WindowIsValidOnConstruction()
				{
					// INIT
					shared_ptr<ut::TestNativeWidget> w(new ut::TestNativeWidget);

					// ACT / ASSERT
					Assert::IsTrue(!!::IsWindow(w->hwnd()));
				}

				
				[TestMethod]
				void WindowIsDestroyedAtWidgetDeleted()
				{
					// INIT
					shared_ptr<ut::TestNativeWidget> w(new ut::TestNativeWidget);
					HWND hwnd = w->hwnd();

					// ACT
					w = shared_ptr<ut::TestNativeWidget>();
					
					// ASSERT
					Assert::IsFalse(!!::IsWindow(hwnd));
				}


				[TestMethod]
				void SettingParentOnNativeViewChangesTestNativeWidgetsParentWindow()
				{
					// INIT
					HWND hparent1 = create_window(), hparent2 = create_window();
					shared_ptr<ut::TestNativeWidget> w(new ut::TestNativeWidget);
					shared_ptr<view> v;

					// ACT
					v = w->create_view(native_root(hparent1));

					// ASSERT
					Assert::IsTrue(::GetParent(w->hwnd()) == hparent1);

					// ACT
					v = w->create_view(native_root(hparent2));

					// ASSERT
					Assert::IsTrue(::GetParent(w->hwnd()) == hparent2);
				}


				[TestMethod]
				void WidgetIsHeldByItsView()
				{
					// INIT
					shared_ptr<widget> w_strong(new ut::TestNativeWidget);
					weak_ptr<widget> w_weak(w_strong);
					shared_ptr<view> v(w_strong->create_view(native_root(0)));

					// ACT
					w_strong = shared_ptr<widget>();

					// ASSERT
					Assert::IsFalse(w_weak.expired());
				}


				[TestMethod]
				void WidgetIsDestroyedOnJunctionDestroy()
				{
					// INIT
					shared_ptr<widget> w_strong(new ut::TestNativeWidget);
					weak_ptr<widget> w_weak(w_strong);
					shared_ptr<view> v(w_strong->create_view(native_root(0)));

					// ACT
					w_strong = shared_ptr<widget>();
					v = shared_ptr<view>();

					// ASSERT
					Assert::IsTrue(w_weak.expired());
				}


				[TestMethod]
				void MoveNativeView()
				{
					// INIT
					HWND hparent = create_window();
					shared_ptr<ut::TestNativeWidget> w(new ut::TestNativeWidget);
					shared_ptr<view> v(w->create_view(native_root(hparent)));
					RECT rc;

					// ACT
					v->move(1, 3, 10, 20);

					// ASSERT
					rc = ut::get_window_rect(w->hwnd());

					Assert::IsTrue(1 == rc.left);
					Assert::IsTrue(3 == rc.top);
					Assert::IsTrue(11 == rc.right);
					Assert::IsTrue(23 == rc.bottom);

					// ACT
					v->move(7, 11, 13, 29);

					// ASSERT
					rc = ut::get_window_rect(w->hwnd());

					Assert::IsTrue(7 == rc.left);
					Assert::IsTrue(11 == rc.top);
					Assert::IsTrue(20 == rc.right);
					Assert::IsTrue(40 == rc.bottom);
				}
			};
		}
	}
}
