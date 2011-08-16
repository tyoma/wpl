#include <wpl/ui/listview.h>
#include <wpl/ui/win32/controls.h>

#include "TestHelpers.h"
#include <windows.h>
#include <commctrl.h>

using namespace std;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace
			{
				class test_model_1 : public listview::model
				{
					index_type _count;

					virtual index_type get_count() const throw()	{	return _count;	}
					virtual void get_text(index_type item, index_type subitem, std::wstring &text) const	{	}
					virtual void set_order(index_type column, bool ascending)	{	}

				public:
					test_model_1(index_type count)
						: _count(count)
					{	}

					void set_count(index_type new_count)
					{
						_count = new_count;
						invalidated(new_count);
					}
				};

				typedef shared_ptr<test_model_1> model1_ptr;
			}

			[TestClass]
			public ref class ListViewTests : ut::WindowTestsBase
			{
				HWND create_listview()
				{	return reinterpret_cast<HWND>(create_window(_T("SysListView32"), create_window(), WS_CHILD | LVS_REPORT | LVS_OWNERDATA, 0));	}

			public:
				[TestMethod]
				void WrongWrappingLeadsToException()
				{
					// INIT
					void *wrong_hwnd1 = (void *)0x12345678;
					void *wrong_hwnd2 = create_window();

					destroy_window(wrong_hwnd2);

					// ACT / ASSERT
					ASSERT_THROWS(wrap_listview(wrong_hwnd1), invalid_argument);
					ASSERT_THROWS(wrap_listview(wrong_hwnd2), invalid_argument);
				}


				[TestMethod]
				void WrapListViewGetNonNullPtr()
				{
					// INIT
					void *hlv = create_window(_T("SysListView32"));

					// ACT / ASSERT
					Assert::IsTrue(wrap_listview(hlv) != 0);
				}


				[TestMethod]
				void ListViewItemsCountSetOnSettingModel()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));

					// ACT
					lv->set_model(model1_ptr(new test_model_1(11)));

					// ASSERT
					Assert::IsTrue(ListView_GetItemCount(hlv) == 11);

					// ACT
					lv->set_model(model1_ptr(new test_model_1(23)));

					// ASSERT
					Assert::IsTrue(ListView_GetItemCount(hlv) == 23);
				}


				[TestMethod]
				void ChangingItemsCountIsTrackedByListView()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model1_ptr m(new test_model_1(0));

					lv->set_model(m);

					// ACT
					m->set_count(3);

					// ASSERT
					Assert::IsTrue(ListView_GetItemCount(hlv) == 3);

					// ACT
					m->set_count(13);

					// ASSERT
					Assert::IsTrue(ListView_GetItemCount(hlv) == 13);
				}
			};
		}
	}
}
