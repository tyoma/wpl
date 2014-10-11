#include <wpl/ui/form.h>

#include <wpl/ui/listview.h>

#include "Mockups.h"
#include "TestHelpers.h"

#include <windows.h>

using namespace Microsoft::VisualStudio::TestTools::UnitTesting;
using namespace std;
using namespace std::tr1::placeholders;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			typedef pair<shared_ptr<form>, HWND> form_and_handle;

			form_and_handle create_form_with_handle();

			[TestClass]
			public ref class ContainerTests : ut::WindowTestsBase
			{
			public:
				[TestMethod]
				void ConstructNestedContainer()
				{
					// INIT
					shared_ptr<form> f = form::create();
					shared_ptr<container> root = f->get_root_container();
					ut::window_tracker wt;

					// ACT
					shared_ptr<container> nc1 = root->create_container(L"1");
					shared_ptr<container> nc2 = root->create_container(L"2");

					// ASSERT
					wt.checkpoint();

					Assert::IsTrue(0 == wt.created.size());
					Assert::IsTrue(!!nc1);
					Assert::IsTrue(!!nc2);
					Assert::IsTrue(nc1 != nc2);

					// ACT
					shared_ptr<container> nc11 = nc1->create_container(L"3");
					shared_ptr<container> nc12 = nc1->create_container(L"4");
					shared_ptr<container> nc21 = nc2->create_container(L"5");

					// ASSERT
					wt.checkpoint();

					Assert::IsTrue(0 == wt.created.size());
					Assert::IsTrue(!!nc11);
					Assert::IsTrue(!!nc12);
					Assert::IsTrue(!!nc21);
				}


				[TestMethod]
				void NestedContainerIsHeldByTheParent()
				{
					// INIT
					shared_ptr<form> f = form::create();
					shared_ptr<container> root = f->get_root_container();

					// ACT
					weak_ptr<container> nc = root->create_container(L"1");

					// ASSERT
					Assert::IsFalse(nc.expired());
				}


				[TestMethod]
				void NestedContainerIsDestroyedAlongWithParent()
				{
					// INIT
					shared_ptr<form> f = form::create();
					weak_ptr<container> nc = f->get_root_container()->create_container(L"1");

					// ACT
					f = shared_ptr<form>();

					// ASSERT
					Assert::IsTrue(nc.expired());
				}


				[TestMethod]
				void NestedChildIsFosteredByTopLevelParent()
				{
					// INIT
					form_and_handle f = create_form_with_handle();
					shared_ptr<container> root = f.first->get_root_container();
					shared_ptr<container> nc1 = root->create_container(L"1");
					shared_ptr<container> nc2 = root->create_container(L"2");
					ut::window_tracker wt(L"SysListView32");

					// ACT
					nc1->create_widget(L"listview", L"1");
					nc2->create_widget(L"listview", L"1");
					wt.checkpoint();

					// ASSERT
					Assert::IsTrue(2u == wt.created.size());
					Assert::IsTrue(f.second == ::GetParent(wt.created[0]));
					Assert::IsTrue(f.second == ::GetParent(wt.created[1]));
				}


				[TestMethod]
				void WidgetsAreEnumeratedOnLayout()
				{
					// INIT
					form_and_handle f[] = {
						create_form_with_handle(),
						create_form_with_handle(),
					};
					shared_ptr<container> c[] = {
						f[0].first->get_root_container(),
						f[1].first->get_root_container()->create_container(L"1"),
					};
					widget_ptr w1[] = {
						c[0]->create_widget(L"listview", L"2"),
						c[0]->create_widget(L"listview", L"3"),
					};
					widget_ptr w2[] = {
						c[1]->create_widget(L"listview", L"2"),
						c[1]->create_widget(L"listview", L"3"),
						c[1]->create_widget(L"listview", L"4"),
					};
					shared_ptr<ut::logging_layout_manager> lm[] = {
						shared_ptr<ut::logging_layout_manager>(new ut::logging_layout_manager),
						shared_ptr<ut::logging_layout_manager>(new ut::logging_layout_manager),
					};

					c[0]->layout = lm[0];
					c[1]->layout = lm[1];
					f[1].first->get_root_container()->layout = shared_ptr<ut::logging_layout_manager>(new ut::logging_layout_manager);

					// ACT
					::MoveWindow(f[0].second, 10, 11, 150, 60, TRUE);
					::MoveWindow(f[1].second, 10, 11, 160, 70, TRUE);

					// ASSERT
					Assert::IsTrue(2u == lm[0]->last_widgets.size());
					Assert::IsTrue(w1[0] == lm[0]->last_widgets[0].first);
					Assert::IsTrue(w1[1] == lm[0]->last_widgets[1].first);
					
					Assert::IsTrue(3u == lm[1]->last_widgets.size());
					Assert::IsTrue(w2[0] == lm[1]->last_widgets[0].first);
					Assert::IsTrue(w2[1] == lm[1]->last_widgets[1].first);
					Assert::IsTrue(w2[2] == lm[1]->last_widgets[2].first);
				}


				[TestMethod]
				void ContainersAreEnumeratedOnLayout()
				{
					// INIT
					form_and_handle f[] = {
						create_form_with_handle(),
						create_form_with_handle(),
					};
					shared_ptr<container> c[] = {
						f[0].first->get_root_container(),
						f[1].first->get_root_container(),
					};
					shared_ptr<container> c1[] = {
						c[0]->create_container(L"2"),
						c[0]->create_container(L"3"),
						c[0]->create_container(L"4"),
					};
					shared_ptr<container> c2[] = {
						c[1]->create_container(L"2"),
						c[1]->create_container(L"3"),
					};
					shared_ptr<ut::logging_layout_manager> lm[] = {
						shared_ptr<ut::logging_layout_manager>(new ut::logging_layout_manager),
						shared_ptr<ut::logging_layout_manager>(new ut::logging_layout_manager),
					};
					shared_ptr<ut::logging_layout_manager> dummy_lm(new ut::logging_layout_manager);

					c[0]->layout = lm[0];
					c[1]->layout = lm[1];
					c1[0]->layout = dummy_lm;
					c1[1]->layout = dummy_lm;
					c1[2]->layout = dummy_lm;
					c2[0]->layout = dummy_lm;
					c2[1]->layout = dummy_lm;

					// ACT
					::MoveWindow(f[0].second, 10, 11, 150, 60, TRUE);
					::MoveWindow(f[1].second, 10, 11, 160, 70, TRUE);

					// ASSERT
					Assert::IsTrue(3u == lm[0]->last_widgets.size());
					Assert::IsTrue(c1[0] == lm[0]->last_widgets[0].first);
					Assert::IsTrue(c1[1] == lm[0]->last_widgets[1].first);
					Assert::IsTrue(c1[2] == lm[0]->last_widgets[2].first);

					Assert::IsTrue(2u == lm[1]->last_widgets.size());
					Assert::IsTrue(c2[0] == lm[1]->last_widgets[0].first);
					Assert::IsTrue(c2[1] == lm[1]->last_widgets[1].first);
				}


				[TestMethod]
				void WidgetsAndContainersAreEnumeratedOnLayout()
				{
					// INIT
					form_and_handle f = create_form_with_handle();
					shared_ptr<container> root = f.first->get_root_container();
					shared_ptr<container> c1 = root->create_container(L"2");
					widget_ptr w2 = root->create_widget(L"listview", L"3");
					shared_ptr<container> c3 = root->create_container(L"4");
					shared_ptr<ut::logging_layout_manager> lm(new ut::logging_layout_manager);
					shared_ptr<ut::logging_layout_manager> dummy_lm(new ut::logging_layout_manager);

					root->layout = lm;
					c1->layout = dummy_lm;
					c3->layout = dummy_lm;

					// ACT
					::MoveWindow(f.second, 10, 11, 150, 60, TRUE);

					// ASSERT
					Assert::IsTrue(3u == lm->last_widgets.size());
					Assert::IsTrue(c1 == lm->last_widgets[0].first);
					Assert::IsTrue(w2 == lm->last_widgets[1].first);
					Assert::IsTrue(c3 == lm->last_widgets[2].first);
				}


				[TestMethod]
				void ChildrenAreRepositionedAccordinglyToTheContainerOffset()
				{
					// INIT
					form_and_handle f = create_form_with_handle();
					ut::window_tracker wt(L"SysListView32");
					shared_ptr<container> root = f.first->get_root_container();
					shared_ptr<container> c1 = root->create_container(L"1");
					widget_ptr w11 = create_widget(wt, *c1, L"listview", L"2");
					widget_ptr w12 = create_widget(wt, *c1, L"listview", L"3");
					widget_ptr w2 = create_widget(wt, *root, L"listview", L"4");
					shared_ptr<container> c3 = root->create_container(L"5");
					widget_ptr w31 = create_widget(wt, *c3, L"listview", L"6");
					widget_ptr w32 = create_widget(wt, *c3, L"listview", L"7");
					widget_ptr w33 = create_widget(wt, *c3, L"listview", L"8");

					wt.checkpoint();
					shared_ptr<ut::logging_layout_manager> lm(new ut::logging_layout_manager);
					shared_ptr<ut::logging_layout_manager> lm1(new ut::logging_layout_manager);
					shared_ptr<ut::logging_layout_manager> lm3(new ut::logging_layout_manager);

					root->layout = lm;
					c1->layout = lm1;
					c3->layout = lm3;

					layout_manager::position p10[] = {
						{ 5, 7, 10, 10 },
						{ 5, 37, 85, 30 },
						{ 5, 70, 10, 10 },
					};
					layout_manager::position p11[] = {
						{ 0, 0, 30, 25 },
						{ 35, 0, 50, 25 },
					};
					layout_manager::position p13[] = {
						{ 1, 2, 45, 35 },
						{ 51, 2, 50, 35 },
						{ 106, 2, 50, 35 },
					};

					lm->positions.assign(ut::begin(p10), ut::end(p10));
					lm1->positions.assign(ut::begin(p11), ut::end(p11));
					lm3->positions.assign(ut::begin(p13), ut::end(p13));

					// ACT
					::MoveWindow(f.second, 10, 11, 150, 60, TRUE);

					// ASSERT
					Assert::IsTrue(ut::rect(5, 7, 30, 25) == ut::get_window_rect(wt.created[0]));
					Assert::IsTrue(ut::rect(40, 7, 50, 25) == ut::get_window_rect(wt.created[1]));
					Assert::IsTrue(ut::rect(5, 37, 85, 30) == ut::get_window_rect(wt.created[2]));
					Assert::IsTrue(ut::rect(6, 72, 45, 35) == ut::get_window_rect(wt.created[3]));
					Assert::IsTrue(ut::rect(56, 72, 50, 35) == ut::get_window_rect(wt.created[4]));
					Assert::IsTrue(ut::rect(111, 72, 50, 35) == ut::get_window_rect(wt.created[5]));
				}
			};
		}
	}
}
