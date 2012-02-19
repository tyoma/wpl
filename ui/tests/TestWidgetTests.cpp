#include "TestWidgets.h"

#include <wpl/ui/geometry.h>
#include <wpl/ui/win32/native_view.h>

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
			public ref class TestWidgetTests : ut::WindowTestsBase
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
				void WidgetProvidesCustomView()
				{
					// INIT
					shared_ptr<widget> w(new ut::TestNativeWidget);

					// ACT
					shared_ptr<view> v(w->create_custom_view());

					// ASSERT
					Assert::IsTrue(!!v);
				}


				[TestMethod]
				void SettingParentOnNativeViewChangesTestNativeWidgetsParentWindow()
				{
					// INIT
					HWND hparent1 = create_window(), hparent2 = create_window();
					shared_ptr<ut::TestNativeWidget> w(new ut::TestNativeWidget);
					shared_ptr<view> v(w->create_custom_view());

					// ACT
					((native_view &)*v).set_parent(native_view::transform_chain(), hparent1);

					// ASSERT
					Assert::IsTrue(::GetParent(w->hwnd()) == hparent1);

					// ACT
					((native_view &)*v).set_parent(native_view::transform_chain(), hparent2);

					// ASSERT
					Assert::IsTrue(::GetParent(w->hwnd()) == hparent2);
				}


				[TestMethod]
				void WidgetIsHeldByItsView()
				{
					// INIT
					shared_ptr<widget> w_strong(new ut::TestNativeWidget);
					weak_ptr<widget> w_weak(w_strong);
					shared_ptr<view> v(w_strong->create_custom_view());

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
					shared_ptr<view> v(w_strong->create_custom_view());

					// ACT
					w_strong = shared_ptr<widget>();
					v = shared_ptr<view>();

					// ASSERT
					Assert::IsTrue(w_weak.expired());
				}


				[TestMethod]
				void MoveWidgetWithEmptyTransformationChain()
				{
					// INIT
					HWND hparent = create_window();
					shared_ptr<ut::TestNativeWidget> w(new ut::TestNativeWidget);
					shared_ptr<view> v(w->create_custom_view());
					RECT rc;

					// ACT
					((native_view &)*v).set_parent(native_view::transform_chain(), hparent);
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


				[TestMethod]
				void MoveWidgetWithSingleZeroOffsetTransformationInTransformationChain()
				{
					// INIT
					HWND hparent = create_window();
					shared_ptr<ut::TestNativeWidget> w(new ut::TestNativeWidget);
					native_view::transform_chain tc(1, shared_ptr<transform>(new transform));
					shared_ptr<view> v(w->create_custom_view());

					// ACT
					((native_view &)*v).set_parent(tc, hparent);
					v->move(1, 3, 10, 20);

					// ASSERT
					RECT rc = ut::get_window_rect(w->hwnd());

					Assert::IsTrue(1 == rc.left);
					Assert::IsTrue(3 == rc.top);
					Assert::IsTrue(11 == rc.right);
					Assert::IsTrue(23 == rc.bottom);
				}


				[TestMethod]
				void MoveWidgetWithSingleTransformationInTransformationChain()
				{
					// INIT
					HWND hparent = create_window();
					shared_ptr<transform> t(new transform);
					native_view::transform_chain tc(1, t);
					shared_ptr<ut::TestNativeWidget> w(new ut::TestNativeWidget);
					shared_ptr<view> v(w->create_custom_view());
					RECT rc;

					// ACT
					((native_view &)*v).set_parent(tc, hparent);

					t->set_origin(101, 220);
					v->move(1, 3, 10, 20);

					// ASSERT
					rc = ut::get_window_rect(w->hwnd());

					Assert::IsTrue(102 == rc.left);
					Assert::IsTrue(223 == rc.top);
					Assert::IsTrue(112 == rc.right);
					Assert::IsTrue(243 == rc.bottom);

					// ACT
					t->set_origin(47, 37);
					v->move(0, 0, 13, 29);

					// ASSERT
					rc = ut::get_window_rect(w->hwnd());

					Assert::IsTrue(47 == rc.left);
					Assert::IsTrue(37 == rc.top);
					Assert::IsTrue(60 == rc.right);
					Assert::IsTrue(66 == rc.bottom);
				}


				[TestMethod]
				void MoveWidgetWithThreeTransformationInTransformationChain()
				{
					// INIT
					HWND hparent = create_window();
					shared_ptr<transform> t[] = {
						shared_ptr<transform>(new transform),
						shared_ptr<transform>(new transform),
						shared_ptr<transform>(new transform),
					};
					native_view::transform_chain tc(t, t + 3);
					shared_ptr<ut::TestNativeWidget> w(new ut::TestNativeWidget);
					shared_ptr<view> v(w->create_custom_view());

					// ACT
					((native_view &)*v).set_parent(tc, hparent);

					t[0]->set_origin(1, 1);
					t[1]->set_origin(2, 3);
					t[2]->set_origin(5, 8);

					v->move(81, 95, 32, 76);

					// ASSERT
					RECT rc = ut::get_window_rect(w->hwnd());

					Assert::IsTrue(89 == rc.left);
					Assert::IsTrue(107 == rc.top);
					Assert::IsTrue(121 == rc.right);
					Assert::IsTrue(183 == rc.bottom);
				}
			};
		}
	}
}
