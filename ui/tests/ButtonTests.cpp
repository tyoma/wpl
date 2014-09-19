#include <wpl/ui/button.h>

#include <wpl/ui/form.h>

#include "Mockups.h"
#include "TestHelpers.h"

#include <windows.h>

using namespace Microsoft::VisualStudio::TestTools::UnitTesting;
using namespace std;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace
			{
				widget_ptr create_widget(ut::window_tracker &wt, container &c, const wstring &type, const wstring &id)
				{
					widget_ptr w = c.create_widget(type, id);

					wt.checkpoint();
					return w;
				}

				void increment(int *counter)
				{	++*counter;	}
			}

			[TestClass]
			public ref class ButtonTests : ut::WindowTestsBase
			{
			public:
				[TestMethod]
				void CreateButton()
				{
					// INIT
					shared_ptr<form> f = form::create();
					ut::window_tracker wt(L"button");

					// ACT
					shared_ptr<button> b = static_pointer_cast<button>(create_widget(wt, *f->get_root_container(), L"button", L"1"));

					// ASSERT
					Assert::IsTrue(!!b);
					Assert::IsTrue(1u == wt.created.size());
				}


				[TestMethod]
				void ReceiveButtonClickOnMouseDownUp()
				{
					// INIT
					shared_ptr<form> f = form::create();
					ut::window_tracker wt(L"button");
					shared_ptr<button> b = static_pointer_cast<button>(create_widget(wt, *f->get_root_container(), L"button", L"1"));
					int click_count = 0;
					slot_connection c = b->clicked += bind(&increment, &click_count);

					f->get_root_container()->layout.reset(new ut::fill_layout);

					// ACT
					::SendMessage(wt.created[0], WM_LBUTTONDOWN, 0, 0);

					// ASSERT
					Assert::IsTrue(0 == click_count);

					// ACT
					::SendMessage(wt.created[0], WM_LBUTTONUP, 0, 0);

					// ASSERT
					Assert::IsTrue(1 == click_count);
				}


				[TestMethod]
				void SettingTextUpdatesButtonWindowText()
				{
					// INIT
					shared_ptr<form> f = form::create();
					ut::window_tracker wt(L"button");
					shared_ptr<button> b = static_pointer_cast<button>(create_widget(wt, *f->get_root_container(), L"button", L"1"));

					// ACT
					b->set_text(L"Start doing something!");

					// ASSERT
					Assert::IsTrue(L"Start doing something!" == ut::get_window_text(wt.created[0]));

					// ACT
					b->set_text(L"Stop that!");

					// ASSERT
					Assert::IsTrue(L"Stop that!" == ut::get_window_text(wt.created[0]));
				}
			};
		}
	}
}
