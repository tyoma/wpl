#include <wpl/ui/listview.h>
#include <wpl/ui/win32/controls.h>

#include "TestHelpers.h"
#include <windows.h>
#include <commctrl.h>
#include <olectl.h>

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
				class test_model : public listview::model
				{
					virtual index_type get_count() const throw()
					{	return items.size();	}

					virtual void get_text(index_type item, index_type subitem, wstring &text) const
					{
						if (item >= items.size())
							Assert::Fail("Requested item is at invalid index!");
						text = items[item][subitem];
					}

					virtual void set_order(index_type column, bool ascending)
					{	ordering.push_back(make_pair(column, ascending));	}

				public:
					test_model(index_type count)
					{	items.resize(count);	}

					void set_count(index_type new_count)
					{
						items.resize(new_count);
						invalidated(items.size());
					}

					vector< vector<wstring> > items;
					vector< pair<index_type, bool> > ordering;
				};

				typedef shared_ptr<test_model> model_ptr;
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
					lv->set_model(model_ptr(new test_model(11)));

					// ASSERT
					Assert::IsTrue(ListView_GetItemCount(hlv) == 11);

					// ACT
					lv->set_model(model_ptr(new test_model(23)));

					// ASSERT
					Assert::IsTrue(ListView_GetItemCount(hlv) == 23);
				}


				[TestMethod]
				void ChangingItemsCountIsTrackedByListView()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new test_model(0));

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


				[TestMethod]
				void GettingDispInfoOtherThanTextIsNotFailing()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new test_model(0));
					TCHAR buffer[100] = { 0 };
					NMLVDISPINFO nmlvdi = {
						{	0, 0, LVN_GETDISPINFO	},
						{ /* mask = */ LVIF_STATE, /* item = */ 0, /* subitem = */ 0, 0, 0, 0, 0, }
					};

					m->set_count(1), m->items[0].resize(3);
					m->items[0][0] = L"one", m->items[0][1] = L"two", m->items[0][2] = L"three";
					lv->set_model(m);

					// ACT / ASSERT (must not throw)
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
				}


				[TestMethod]
				void GettingItemTextNoTruncation()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new test_model(0));
					TCHAR buffer[100] = { 0 };
					NMLVDISPINFO nmlvdi = {
						{	0, 0, LVN_GETDISPINFO	},
						{ /* mask = */ LVIF_TEXT, /* item = */ 0, /* subitem = */ 0, 0, 0, buffer, sizeof(buffer) / sizeof(TCHAR), }
					};

					m->set_count(3), m->items[0].resize(3), m->items[1].resize(3), m->items[2].resize(3);
					m->items[0][0] = L"one", m->items[0][1] = L"two", m->items[0][2] = L"three";
					m->items[1][0] = L"four", m->items[1][1] = L"five", m->items[1][2] = L"six";
					m->items[2][0] = L"seven", m->items[2][1] = L"eight", m->items[2][2] = L"NINE";
					lv->set_model(m);

					// ACT / ASSERT
					nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 0;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == _tcscmp(_T("one"), buffer));

					nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 1;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == _tcscmp(_T("two"), buffer));

					nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 2;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == _tcscmp(_T("three"), buffer));

					nmlvdi.item.iItem = 1, nmlvdi.item.iSubItem = 0;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == _tcscmp(_T("four"), buffer));

					nmlvdi.item.iItem = 1, nmlvdi.item.iSubItem = 1;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == _tcscmp(_T("five"), buffer));

					nmlvdi.item.iItem = 1, nmlvdi.item.iSubItem = 2;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == _tcscmp(_T("six"), buffer));

					nmlvdi.item.iItem = 2, nmlvdi.item.iSubItem = 0;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == _tcscmp(_T("seven"), buffer));

					nmlvdi.item.iItem = 2, nmlvdi.item.iSubItem = 1;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == _tcscmp(_T("eight"), buffer));

					nmlvdi.item.iItem = 2, nmlvdi.item.iSubItem = 2;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == _tcscmp(_T("NINE"), buffer));
				}


				[TestMethod]
				void InitialOrdering()
				{
					// INIT
					HWND hlv1 = create_listview(), hlv2 = create_listview(), hlv3 = create_listview();
					shared_ptr<listview> lv1(wrap_listview(hlv1)), lv2(wrap_listview(hlv2)), lv3(wrap_listview(hlv3));
					model_ptr m(new test_model(0));
					NMLISTVIEW nmlvdi = {
						{	0, 0, LVN_COLUMNCLICK	},
						/* iItem = */ LVIF_TEXT /* see if wndproc differentiates notifications */, /* iSubItem = */ 0,
					};

					lv1->set_model(m);
					lv2->set_model(m);
					lv3->set_model(m);
					lv1->add_column(L"", listview::dir_none);
					lv2->add_column(L"", listview::dir_ascending);
					lv2->add_column(L"", listview::dir_ascending);
					lv3->add_column(L"", listview::dir_descending);
					lv3->add_column(L"", listview::dir_descending);
					lv3->add_column(L"", listview::dir_descending);

					// ACT
					nmlvdi.iSubItem = 0;
					::SendMessage(hlv1, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

					// ASSERT
					Assert::IsTrue(0 == m->ordering.size());

					// ACT
					nmlvdi.iSubItem = 0;
					::SendMessage(hlv2, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

					// ASSERT
					Assert::IsTrue(1 == m->ordering.size());
					Assert::IsTrue(0 == m->ordering[0].first);
					Assert::IsTrue(true == m->ordering[0].second);

					// ACT
					nmlvdi.iSubItem = 1;
					::SendMessage(hlv2, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

					// ASSERT
					Assert::IsTrue(2 == m->ordering.size());
					Assert::IsTrue(1 == m->ordering[1].first);
					Assert::IsTrue(true == m->ordering[1].second);

					// ACT
					nmlvdi.iSubItem = 2;
					::SendMessage(hlv3, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

					// ASSERT
					Assert::IsTrue(3 == m->ordering.size());
					Assert::IsTrue(2 == m->ordering[2].first);
					Assert::IsTrue(false == m->ordering[2].second);
				}
			};
		}
	}
}
