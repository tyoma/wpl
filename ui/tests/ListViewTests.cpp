#include <wpl/ui/listview.h>
#include <wpl/ui/win32/controls.h>

#include "TestHelpers.h"
#include <windows.h>
#include <commctrl.h>
#include <olectl.h>

using namespace std;
using namespace tr1;
using namespace tr1::placeholders;
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

				template <typename T>
				void push_back(vector<T> &v, const T &value)
				{	v.push_back(value);	}

				listview::sort_direction get_column_direction(void *hlv, listview::index_type column)
				{
					HDITEM item = { 0 };
					HWND hheader = ListView_GetHeader(reinterpret_cast<HWND>(hlv));
					
					item.mask = HDI_FORMAT;
					Header_GetItem(hheader, column, &item);
					return (item.fmt & HDF_SORTUP) ? listview::dir_ascending : (item.fmt & HDF_SORTDOWN) ?
						listview::dir_descending : listview::dir_none;
				}
			}

			[TestClass]
			public ref class ListViewTests : ut::WindowTestsBase
			{
				HWND create_listview()
				{
					return reinterpret_cast<HWND>(create_window(_T("SysListView32"), create_window(),
						WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_OWNERDATA, 0));
				}

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
				void ListViewInvalidatedOnModelInvalidate()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new test_model(11));

					lv->add_column(L"test", listview::dir_none);
					lv->set_model(m);

					// ACT
					::UpdateWindow(hlv);

					// ASSERT
					Assert::IsFalse(!!::GetUpdateRect(hlv, NULL, FALSE));

					// ACT
					m->invalidated(11);

					// ASSERT
					Assert::IsTrue(!!::GetUpdateRect(hlv, NULL, FALSE));
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
				void InvalidationsFromOldModelsAreNotAccepted()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m1(new test_model(5)), m2(new test_model(7));

					lv->set_model(m1);

					// ACT
					lv->set_model(m2);
					m1->set_count(3);

					// ASSERT
					Assert::IsTrue(7 == ListView_GetItemCount(hlv));
				}


				[TestMethod]
				void ResettingModelSetsZeroItemsCountAndDisconnectsFromInvalidationEvent()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new test_model(5));

					lv->set_model(m);

					// ACT
					lv->set_model(0);

					// ASSERT
					Assert::IsTrue(0 == ListView_GetItemCount(hlv));

					// ACT
					m->set_count(7);

					// ASSERT
					Assert::IsTrue(0 == ListView_GetItemCount(hlv));
				}


				[TestMethod]
				void GettingDispInfoOtherThanTextIsNotFailing()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new test_model(0));
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
				void GettingItemTextWithTruncation()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new test_model(0));
					TCHAR buffer[4] = { 0 };
					NMLVDISPINFO nmlvdi = {
						{	0, 0, LVN_GETDISPINFO	},
						{ /* mask = */ LVIF_TEXT, /* item = */ 0, /* subitem = */ 0, 0, 0, buffer, sizeof(buffer) / sizeof(TCHAR), }
					};

					m->set_count(3), m->items[0].resize(3);
					m->items[0][0] = L"one", m->items[0][1] = L"two", m->items[0][2] = L"three";
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
					Assert::IsTrue(0 == _tcscmp(_T("thr"), buffer));
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


				[TestMethod]
				void PreorderingOfANewModel()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m1(new test_model(0)), m2(new test_model(0));
					NMLISTVIEW nmlvdi = {
						{	0, 0, LVN_COLUMNCLICK	},
						/* iItem = */ 0, /* iSubItem = */ 0,
					};

					lv->set_model(m1);
					lv->add_column(L"", listview::dir_ascending);
					lv->add_column(L"", listview::dir_descending);
					nmlvdi.iSubItem = 0;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

					// ACT
					lv->set_model(m2);

					// ASSERT
					Assert::IsTrue(1 == m2->ordering.size());
					Assert::IsTrue(0 == m2->ordering[0].first);
					Assert::IsTrue(m2->ordering[0].second);

					// INIT
					nmlvdi.iSubItem = 1;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

					// ACT
					lv->set_model(m1);

					// ASSERT
					Assert::IsTrue(2 == m1->ordering.size());
					Assert::IsTrue(1 == m1->ordering[1].first);
					Assert::IsFalse(m1->ordering[1].second);
				}


				[TestMethod]
				void PreorderingHandlesNullModelWell()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new test_model(0));
					NMLISTVIEW nmlvdi = {
						{	0, 0, LVN_COLUMNCLICK	},
						/* iItem = */ 0, /* iSubItem = */ 0,
					};

					lv->set_model(m);
					lv->add_column(L"", listview::dir_ascending);
					lv->add_column(L"", listview::dir_descending);
					nmlvdi.iSubItem = 0;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

					// ACT / ASSERT (must not throw)
					lv->set_model(0);
				}


				[TestMethod]
				void OrderReversing()
				{
					// INIT
					HWND hlv1 = create_listview(), hlv2 = create_listview();
					shared_ptr<listview> lv1(wrap_listview(hlv1)), lv2(wrap_listview(hlv2));
					model_ptr m(new test_model(0));
					NMLISTVIEW nmlvdi = {
						{	0, 0, LVN_COLUMNCLICK	},
						/* iItem = */ 0, /* iSubItem = */ 0,
					};

					lv1->set_model(m);
					lv2->set_model(m);
					lv1->add_column(L"", listview::dir_ascending);
					lv1->add_column(L"", listview::dir_ascending);
					lv2->add_column(L"", listview::dir_descending);
					lv2->add_column(L"", listview::dir_descending);
					lv2->add_column(L"", listview::dir_descending);
					nmlvdi.iSubItem = 0;
					::SendMessage(hlv1, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					nmlvdi.iSubItem = 2;
					::SendMessage(hlv2, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

					m->ordering.clear();

					// ACT
					nmlvdi.iSubItem = 0;
					::SendMessage(hlv1, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

					// ASSERT
					Assert::IsTrue(1 == m->ordering.size());
					Assert::IsTrue(0 == m->ordering[0].first);
					Assert::IsTrue(false == m->ordering[0].second);

					// ACT
					nmlvdi.iSubItem = 2;
					::SendMessage(hlv2, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					nmlvdi.iSubItem = 2;
					::SendMessage(hlv2, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

					// ASSERT
					Assert::IsTrue(3 == m->ordering.size());
					Assert::IsTrue(2 == m->ordering[1].first);
					Assert::IsTrue(true == m->ordering[1].second);
					Assert::IsTrue(2 == m->ordering[2].first);
					Assert::IsTrue(false == m->ordering[2].second);
				}


				[TestMethod]
				void OrderDefaultingAfterSwitchigSortColumn()
				{
					// INIT
					HWND hlv1 = create_listview(), hlv2 = create_listview();
					shared_ptr<listview> lv1(wrap_listview(hlv1)), lv2(wrap_listview(hlv2));
					model_ptr m(new test_model(0));
					NMLISTVIEW nmlvdi = {
						{	0, 0, LVN_COLUMNCLICK	},
						/* iItem = */ 0, /* iSubItem = */ 0,
					};

					lv1->set_model(m);
					lv2->set_model(m);
					lv1->add_column(L"", listview::dir_ascending);
					lv1->add_column(L"", listview::dir_ascending);
					lv2->add_column(L"", listview::dir_descending);
					lv2->add_column(L"", listview::dir_ascending);
					lv2->add_column(L"", listview::dir_descending);
					lv2->add_column(L"", listview::dir_none);
					nmlvdi.iSubItem = 0;
					::SendMessage(hlv1, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					nmlvdi.iSubItem = 0;
					::SendMessage(hlv2, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

					m->ordering.clear();

					// ACT
					nmlvdi.iSubItem = 1;
					::SendMessage(hlv1, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

					// ASSERT
					Assert::IsTrue(1 == m->ordering.size());
					Assert::IsTrue(1 == m->ordering[0].first);
					Assert::IsTrue(true == m->ordering[0].second);

					// ACT
					nmlvdi.iSubItem = 1;
					::SendMessage(hlv2, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					nmlvdi.iSubItem = 2;
					::SendMessage(hlv2, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

					// ASSERT
					Assert::IsTrue(3 == m->ordering.size());
					Assert::IsTrue(1 == m->ordering[1].first);
					Assert::IsTrue(true == m->ordering[1].second);
					Assert::IsTrue(2 == m->ordering[2].first);
					Assert::IsTrue(false == m->ordering[2].second);

					// ACT
					nmlvdi.iSubItem = 3;
					::SendMessage(hlv2, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

					// ASSERT
					Assert::IsTrue(3 == m->ordering.size());	// clicking non-sorting column changes nothing
				}



				[TestMethod]
				void ColumnMarkerIsSet()
				{
					// INIT
					HWND hlv1 = create_listview(), hlv2 = create_listview();
					shared_ptr<listview> lv1(wrap_listview(hlv1)), lv2(wrap_listview(hlv2));
					model_ptr m(new test_model(0));
					NMLISTVIEW nmlvdi = {
						{	0, 0, LVN_COLUMNCLICK	},
						/* iItem = */ 0, /* iSubItem = */ 0,
					};

					lv1->set_model(m);
					lv2->set_model(m);

					// ACT
					lv1->add_column(L"", listview::dir_descending);
					lv1->add_column(L"", listview::dir_ascending);
					lv2->add_column(L"", listview::dir_ascending);
					lv2->add_column(L"", listview::dir_descending);
					lv2->add_column(L"", listview::dir_none);

					// ASSERT
					Assert::IsTrue(listview::dir_none == get_column_direction(hlv1, 0));
					Assert::IsTrue(listview::dir_none == get_column_direction(hlv1, 1));
					Assert::IsTrue(listview::dir_none == get_column_direction(hlv2, 0));
					Assert::IsTrue(listview::dir_none == get_column_direction(hlv2, 1));
					Assert::IsTrue(listview::dir_none == get_column_direction(hlv2, 2));

					// ACT
					nmlvdi.iSubItem = 0;
					::SendMessage(hlv1, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					::SendMessage(hlv2, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

					// ASSERT
					Assert::IsTrue(listview::dir_descending == get_column_direction(hlv1, 0));
					Assert::IsTrue(listview::dir_none == get_column_direction(hlv1, 1));
					Assert::IsTrue(listview::dir_ascending == get_column_direction(hlv2, 0));
					Assert::IsTrue(listview::dir_none == get_column_direction(hlv2, 1));
					Assert::IsTrue(listview::dir_none == get_column_direction(hlv2, 2));

					// ACT
					nmlvdi.iSubItem = 1;
					::SendMessage(hlv1, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					::SendMessage(hlv2, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

					// ASSERT
					Assert::IsTrue(listview::dir_none == get_column_direction(hlv1, 0));
					Assert::IsTrue(listview::dir_ascending == get_column_direction(hlv1, 1));
					Assert::IsTrue(listview::dir_none == get_column_direction(hlv2, 0));
					Assert::IsTrue(listview::dir_descending == get_column_direction(hlv2, 1));
					Assert::IsTrue(listview::dir_none == get_column_direction(hlv2, 2));

					// ACT
					::SendMessage(hlv1, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					nmlvdi.iSubItem = 2;
					::SendMessage(hlv2, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

					// ASSERT
					Assert::IsTrue(listview::dir_none == get_column_direction(hlv1, 0));
					Assert::IsTrue(listview::dir_descending == get_column_direction(hlv1, 1));
					Assert::IsTrue(listview::dir_none == get_column_direction(hlv2, 0));
					Assert::IsTrue(listview::dir_descending == get_column_direction(hlv2, 1));
					Assert::IsTrue(listview::dir_none == get_column_direction(hlv2, 2));
				}


				[TestMethod]
				void ItemActivationFiresCorrespondingEvent()
				{
					// INIT
					vector<listview::index_type> selections;
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					shared_ptr<destructible> c =
						lv->item_activate += bind(&push_back<listview::index_type>, ref(selections), _1);
					NMITEMACTIVATE nm = {	{	0, 0, LVN_ITEMACTIVATE	},	};

					// ACT
					nm.iItem = 1;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nm));

					// ASSERT
					Assert::IsTrue(1 == selections.size());
					Assert::IsTrue(1 == selections[0]);

					// ACT
					nm.iItem = 0;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nm));
					nm.iItem = 3;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nm));
					nm.iItem = 5;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nm));

					// ASSERT
					Assert::IsTrue(4 == selections.size());
					Assert::IsTrue(0 == selections[1]);
					Assert::IsTrue(3 == selections[2]);
					Assert::IsTrue(5 == selections[3]);
				}


				[TestMethod]
				void ItemChangeWithSelectionRemainingDoesNotFireEvent()
				{
					// INIT
					vector<listview::index_type> selection_indices;
					vector<bool> selection_states;
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					shared_ptr<destructible>
						c1 = lv->selection_changed += bind(&push_back<listview::index_type>, ref(selection_indices), _1),
						c2 = lv->selection_changed += bind(&push_back<bool>, ref(selection_states), _2);
					NMLISTVIEW nmlv = {
						{	0, 0, LVN_ITEMCHANGED	},
						/* iItem = */ 0, /* iSubItem = */ 0,
					};

					// ACT
					nmlv.iItem = 0, nmlv.uOldState = LVIS_FOCUSED, nmlv.uNewState = 0;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
					nmlv.iItem = 1, nmlv.uOldState = 0, nmlv.uNewState = LVIS_FOCUSED;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
					nmlv.iItem = 3, nmlv.uOldState = LVIS_FOCUSED | LVIS_SELECTED, nmlv.uNewState = LVIS_SELECTED;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));

					// ASSERT
					Assert::IsTrue(selection_indices.empty());
				}


				[TestMethod]
				void ItemChangeWithSelectionChangingDoesFireEvent()
				{
					// INIT
					vector<listview::index_type> selection_indices;
					vector<bool> selection_states;
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					shared_ptr<destructible>
						c1 = lv->selection_changed += bind(&push_back<listview::index_type>, ref(selection_indices), _1),
						c2 = lv->selection_changed += bind(&push_back<bool>, ref(selection_states), _2);
					NMLISTVIEW nmlv = {
						{	0, 0, LVN_ITEMCHANGED	},
						/* iItem = */ 0, /* iSubItem = */ 0,
					};

					// ACT
					nmlv.iItem = 1, nmlv.uOldState = LVIS_FOCUSED | LVIS_SELECTED, nmlv.uNewState = 0;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
					nmlv.iItem = 2, nmlv.uOldState = LVIS_SELECTED, nmlv.uNewState = LVIS_FOCUSED;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
					nmlv.iItem = 7, nmlv.uOldState = LVIS_FOCUSED, nmlv.uNewState = LVIS_FOCUSED | LVIS_SELECTED;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));

					// ASSERT
					Assert::IsTrue(3 == selection_indices.size());
					Assert::IsTrue(3 == selection_states.size());
					Assert::IsTrue(1 == selection_indices[0]);
					Assert::IsFalse(selection_states[0]);
					Assert::IsTrue(2 == selection_indices[1]);
					Assert::IsFalse(selection_states[1]);
					Assert::IsTrue(7 == selection_indices[2]);
					Assert::IsTrue(selection_states[2]);

					// ACT
					nmlv.iItem = 9, nmlv.uOldState = 0, nmlv.uNewState = LVIS_SELECTED;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));

					// ASSERT
					Assert::IsTrue(4 == selection_indices.size());
					Assert::IsTrue(4 == selection_states.size());
					Assert::IsTrue(9 == selection_indices[3]);
					Assert::IsTrue(selection_states[3]);
				}
			};
		}
	}
}
