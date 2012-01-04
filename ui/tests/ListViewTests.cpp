#include <wpl/ui/listview.h>
#include <wpl/ui/win32/controls.h>

#include "TestHelpers.h"
#include <windows.h>
#include <commctrl.h>
#include <olectl.h>
#include <atlstr.h>
#include <map>

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
				struct mock_trackable : public listview::trackable
				{
					~mock_trackable()
					{	}

					virtual listview::index_type index() const
					{	return track_result;	}

					listview::index_type track_result;

					template <typename Map>
					static shared_ptr<mock_trackable> add(Map &m, listview::index_type index)
					{
						shared_ptr<mock_trackable> t(new mock_trackable());

						m[t->track_result = index] = t;
						return t;
					}
				};

				class mock_model : public listview::model
				{
					virtual index_type get_count() const throw()
					{	return items.size();	}

					virtual void get_text(index_type row, index_type column, wstring &text) const
					{
						if (row >= items.size())
							Assert::Fail("Requested item is at invalid index!");
						text = items[row][column];
					}

					virtual void set_order(index_type column, bool ascending)
					{	ordering.push_back(make_pair(column, ascending));	}

					virtual shared_ptr<const listview::trackable> track(index_type row) const
					{
						tracking_requested.push_back(row);

						map< index_type, shared_ptr<const listview::trackable> >::const_iterator i = trackables.find(row);

						return i != trackables.end() ? i->second : shared_ptr<const listview::trackable>();
					}

				public:
					mock_model(index_type count, index_type columns = 0)
					{	items.resize(count, vector<wstring>(columns));	}

					void set_count(index_type new_count)
					{
						items.resize(new_count);
						invalidated(items.size());
					}

					vector< vector<wstring> > items;
					map< index_type, shared_ptr<const listview::trackable> > trackables;
					vector< pair<index_type, bool> > ordering;
					mutable vector<index_type> tracking_requested;
				};

				typedef shared_ptr<mock_model> model_ptr;
				typedef shared_ptr<mock_trackable> trackable_ptr;

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

				CString get_column_text(void *hlv, listview::index_type column)
				{
					CString buffer;
					HDITEM item = { 0 };
					HWND hheader = ListView_GetHeader(reinterpret_cast<HWND>(hlv));
					
					item.mask = HDI_TEXT;
					item.pszText = buffer.GetBuffer(item.cchTextMax = 100);
					Header_GetItem(hheader, column, &item);
					buffer.ReleaseBuffer();
					return buffer;
				}

				int get_column_width(void *hlv, listview::index_type column)
				{
					HDITEM item = { 0 };
					HWND hheader = ListView_GetHeader(reinterpret_cast<HWND>(hlv));
					
					item.mask = HDI_WIDTH;
					Header_GetItem(hheader, column, &item);
					return item.cxy;
				}

				vector<int> get_matching_indices(void *hlv, unsigned int mask)
				{
					int i = -1;
					vector<int> result;

					mask &= LVNI_STATEMASK;
					while (i = ListView_GetNextItem(reinterpret_cast<HWND>(hlv), i, LVNI_ALL | mask), i != -1)
						result.push_back(i);
					return result;
				}

				bool is_item_visible(void *hlv, int item)
				{
					RECT rc1, rc2, rc;

					::GetClientRect(reinterpret_cast<HWND>(hlv), &rc1);
					ListView_GetItemRect(reinterpret_cast<HWND>(hlv), item, &rc2, LVIR_BOUNDS);
					return !!IntersectRect(&rc, &rc1, &rc2);
				}
			}

			[TestClass]
			public ref class ListViewTests : ut::WindowTestsBase
			{
				HWND create_listview()
				{
					void *hparent = create_visible_window();

					enable_reflection(hparent);
					return reinterpret_cast<HWND>(create_window(_T("SysListView32"), hparent,
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
					lv->set_model(model_ptr(new mock_model(11)));

					// ASSERT
					Assert::IsTrue(ListView_GetItemCount(hlv) == 11);

					// ACT
					lv->set_model(model_ptr(new mock_model(23)));

					// ASSERT
					Assert::IsTrue(ListView_GetItemCount(hlv) == 23);
				}


				[TestMethod]
				void ListViewInvalidatedOnModelInvalidate()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(11, 1));

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
					model_ptr m(new mock_model(0));

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
					model_ptr m1(new mock_model(5)), m2(new mock_model(7));

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
					model_ptr m(new mock_model(5));

					lv->set_model(m);

					// ACT
					lv->set_model(shared_ptr<listview::model>());

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
					model_ptr m(new mock_model(0));
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
				void GettingItemTextNoTruncationANSI()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(0));
					char buffer[100] = { 0 };
					NMLVDISPINFOA nmlvdi = {
						{	0, 0, LVN_GETDISPINFOA	},
						{ /* mask = */ LVIF_TEXT, /* item = */ 0, /* subitem = */ 0, 0, 0, buffer, sizeof(buffer) / sizeof(buffer[0]), }
					};

					m->set_count(3), m->items[0].resize(3), m->items[1].resize(3), m->items[2].resize(3);
					m->items[0][0] = L"one", m->items[0][1] = L"two", m->items[0][2] = L"three";
					m->items[1][0] = L"four", m->items[1][1] = L"five", m->items[1][2] = L"six";
					m->items[2][0] = L"seven", m->items[2][1] = L"eight", m->items[2][2] = L"NINE";
					lv->set_model(m);

					// ACT / ASSERT
					nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 0;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == strcmp("one", buffer));

					nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 1;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == strcmp("two", buffer));

					nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 2;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == strcmp("three", buffer));

					nmlvdi.item.iItem = 1, nmlvdi.item.iSubItem = 0;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == strcmp("four", buffer));

					nmlvdi.item.iItem = 1, nmlvdi.item.iSubItem = 1;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == strcmp("five", buffer));

					nmlvdi.item.iItem = 1, nmlvdi.item.iSubItem = 2;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == strcmp("six", buffer));

					nmlvdi.item.iItem = 2, nmlvdi.item.iSubItem = 0;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == strcmp("seven", buffer));

					nmlvdi.item.iItem = 2, nmlvdi.item.iSubItem = 1;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == strcmp("eight", buffer));

					nmlvdi.item.iItem = 2, nmlvdi.item.iSubItem = 2;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == strcmp("NINE", buffer));
				}


				[TestMethod]
				void GettingItemTextNoTruncationUnicode()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(0));
					wchar_t buffer[100] = { 0 };
					NMLVDISPINFOW nmlvdi = {
						{	0, 0, LVN_GETDISPINFOW	},
						{ /* mask = */ LVIF_TEXT, /* item = */ 0, /* subitem = */ 0, 0, 0, buffer, sizeof(buffer) / sizeof(buffer[0]), }
					};

					m->set_count(3), m->items[0].resize(3), m->items[1].resize(3), m->items[2].resize(3);
					m->items[0][0] = L"one", m->items[0][1] = L"two", m->items[0][2] = L"three";
					m->items[1][0] = L"four", m->items[1][1] = L"five", m->items[1][2] = L"six";
					m->items[2][0] = L"seven", m->items[2][1] = L"eight", m->items[2][2] = L"NINE";
					lv->set_model(m);

					// ACT / ASSERT
					nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 0;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == wcscmp(L"one", buffer));

					nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 1;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == wcscmp(L"two", buffer));

					nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 2;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == wcscmp(L"three", buffer));

					nmlvdi.item.iItem = 1, nmlvdi.item.iSubItem = 0;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == wcscmp(L"four", buffer));

					nmlvdi.item.iItem = 1, nmlvdi.item.iSubItem = 1;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == wcscmp(L"five", buffer));

					nmlvdi.item.iItem = 1, nmlvdi.item.iSubItem = 2;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == wcscmp(L"six", buffer));

					nmlvdi.item.iItem = 2, nmlvdi.item.iSubItem = 0;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == wcscmp(L"seven", buffer));

					nmlvdi.item.iItem = 2, nmlvdi.item.iSubItem = 1;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == wcscmp(L"eight", buffer));

					nmlvdi.item.iItem = 2, nmlvdi.item.iSubItem = 2;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == wcscmp(L"NINE", buffer));
				}


				[TestMethod]
				void GettingItemTextWithTruncationANSI()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(0));
					char buffer[4] = { 0 };
					NMLVDISPINFOA nmlvdi = {
						{	0, 0, LVN_GETDISPINFOA	},
						{ /* mask = */ LVIF_TEXT, /* item = */ 0, /* subitem = */ 0, 0, 0, buffer, sizeof(buffer) / sizeof(buffer[0]), }
					};

					m->set_count(3), m->items[0].resize(3);
					m->items[0][0] = L"one", m->items[0][1] = L"two", m->items[0][2] = L"three";
					lv->set_model(m);

					// ACT / ASSERT
					nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 0;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == strcmp("one", buffer));

					nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 1;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == strcmp("two", buffer));

					nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 2;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == strcmp("thr", buffer));
				}


				[TestMethod]
				void GettingItemTextWithTruncationUnicode()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(0));
					wchar_t buffer[4] = { 0 };
					NMLVDISPINFOW nmlvdi = {
						{	0, 0, LVN_GETDISPINFOW	},
						{ /* mask = */ LVIF_TEXT, /* item = */ 0, /* subitem = */ 0, 0, 0, buffer, sizeof(buffer) / sizeof(buffer[0]), }
					};

					m->set_count(3), m->items[0].resize(3);
					m->items[0][0] = L"one", m->items[0][1] = L"two", m->items[0][2] = L"three";
					lv->set_model(m);

					// ACT / ASSERT
					nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 0;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == wcscmp(L"one", buffer));

					nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 1;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == wcscmp(L"two", buffer));

					nmlvdi.item.iItem = 0, nmlvdi.item.iSubItem = 2;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));
					Assert::IsTrue(0 == wcscmp(L"thr", buffer));
				}


				[TestMethod]
				void InitialOrdering()
				{
					// INIT
					HWND hlv1 = create_listview(), hlv2 = create_listview(), hlv3 = create_listview();
					shared_ptr<listview> lv1(wrap_listview(hlv1)), lv2(wrap_listview(hlv2)), lv3(wrap_listview(hlv3));
					model_ptr m(new mock_model(0));
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
					model_ptr m1(new mock_model(0)), m2(new mock_model(0));
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
					model_ptr m(new mock_model(0));
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
					lv->set_model(shared_ptr<listview::model>());
				}


				[TestMethod]
				void OrderReversing()
				{
					// INIT
					HWND hlv1 = create_listview(), hlv2 = create_listview();
					shared_ptr<listview> lv1(wrap_listview(hlv1)), lv2(wrap_listview(hlv2));
					model_ptr m(new mock_model(0));
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
					model_ptr m(new mock_model(0));
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
					model_ptr m(new mock_model(0));
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
				void TheWholeListViewIsInvalidatedOnReorder()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(0));
					NMLISTVIEW nmlvdi = {
						{	0, 0, LVN_COLUMNCLICK	},
						/* iItem = */ 0, /* iSubItem = */ 0,
					};
					RECT rc_client = { 0 }, rc_invalidated = { 0 };

					lv->set_model(m);
					lv->add_column(L"", listview::dir_descending);
					::UpdateWindow(hlv);
					::GetClientRect(hlv, &rc_client);

					// ACT
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

					// ASSERT
					::GetUpdateRect(hlv, &rc_invalidated, FALSE);
					Assert::IsTrue(rc_client.left == rc_invalidated.left);
					Assert::IsTrue(rc_client.top == rc_invalidated.top);
					Assert::IsTrue(rc_client.right == rc_invalidated.right);
					Assert::IsTrue(rc_client.bottom == rc_invalidated.bottom);
				}
				

				[TestMethod]
				void TheWholeListViewIsInvalidatedOnNoReorderingColumnClicked()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(0));
					NMLISTVIEW nmlvdi = {
						{	0, 0, LVN_COLUMNCLICK	},
						/* iItem = */ 0, /* iSubItem = */ 0,
					};

					lv->set_model(m);
					lv->add_column(L"", listview::dir_none);
					::UpdateWindow(hlv);

					// ACT
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlvdi));

					// ASSERT
					Assert::IsFalse(!!::GetUpdateRect(hlv, NULL, FALSE));
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

					lv->set_model(model_ptr(new mock_model(10)));

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

					lv->set_model(model_ptr(new mock_model(10)));

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


				[TestMethod]
				void AddingColumnSetsItsText()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));

					// ACT
					lv->add_column(L"First", listview::dir_none);
					lv->add_column(L"Second", listview::dir_none);
					lv->add_column(L"Third", listview::dir_none);

					// ASSERT
					Assert::IsTrue(_T("First") == get_column_text(hlv, 0));
					Assert::IsTrue(_T("Second") == get_column_text(hlv, 1));
					Assert::IsTrue(_T("Third") == get_column_text(hlv, 2));
				}


				[TestMethod]
				void AutoAdjustColumnWidths()
				{
					// INIT
					HWND hlv1 = create_listview(), hlv2 = create_listview();
					shared_ptr<listview> lv1(wrap_listview(hlv1)), lv2(wrap_listview(hlv2));

					lv1->add_column(L"ww", listview::dir_none);
					lv1->add_column(L"wwwwww", listview::dir_none);
					lv1->add_column(L"WW", listview::dir_none);
					lv2->add_column(L"ii", listview::dir_none);
					lv2->add_column(L"iiii", listview::dir_none);

					// ACT
					lv1->adjust_column_widths();
					lv2->adjust_column_widths();

					// ASSERT
					int w10 = get_column_width(hlv1, 0);
					int w11 = get_column_width(hlv1, 1);
					int w12 = get_column_width(hlv1, 2);
					int w20 = get_column_width(hlv2, 0);
					int w21 = get_column_width(hlv2, 1);

					Assert::IsTrue(w10 < w11);
					Assert::IsTrue(w12 < w11);
					Assert::IsTrue(w10 < w12);

					Assert::IsTrue(w20 < w10);
					Assert::IsTrue(w20 < w21);
				}


				[TestMethod]
				void SelectionIsEmptyAtConstruction()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));

					// ACT
					lv->set_model(model_ptr(new mock_model(7)));

					// ASSERT
					Assert::IsTrue(get_matching_indices(hlv, LVNI_SELECTED).empty());
				}


				[TestMethod]
				void ResetSelection()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					vector<int> selection;

					lv->set_model(model_ptr(new mock_model(7)));

					// ACT
					lv->select(0, true);

					// ASSERT
					selection = get_matching_indices(hlv, LVNI_SELECTED);

					Assert::IsTrue(1 == selection.size());
					Assert::IsTrue(0 == selection[0]);

					// ACT
					lv->select(3, true);

					// ASSERT
					selection = get_matching_indices(hlv, LVNI_SELECTED);

					Assert::IsTrue(1 == selection.size());
					Assert::IsTrue(3 == selection[0]);

					// ACT
					lv->select(5, true);

					// ASSERT
					selection = get_matching_indices(hlv, LVNI_SELECTED);

					Assert::IsTrue(1 == selection.size());
					Assert::IsTrue(5 == selection[0]);
				}


				[TestMethod]
				void AppendSelection()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					vector<int> selection;

					lv->set_model(model_ptr(new mock_model(7)));

					// ACT
					lv->select(1, false);

					// ASSERT
					selection = get_matching_indices(hlv, LVNI_SELECTED);

					Assert::IsTrue(1 == selection.size());
					Assert::IsTrue(1 == selection[0]);

					// ACT
					lv->select(2, false);

					// ASSERT
					selection = get_matching_indices(hlv, LVNI_SELECTED);

					Assert::IsTrue(2 == selection.size());
					Assert::IsTrue(1 == selection[0]);
					Assert::IsTrue(2 == selection[1]);

					// ACT
					lv->select(6, false);

					// ASSERT
					selection = get_matching_indices(hlv, LVNI_SELECTED);

					Assert::IsTrue(3 == selection.size());
					Assert::IsTrue(1 == selection[0]);
					Assert::IsTrue(2 == selection[1]);
					Assert::IsTrue(6 == selection[2]);
				}


				[TestMethod]
				void ClearNonEmptySelection()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));

					lv->set_model(model_ptr(new mock_model(7)));

					// ACT
					lv->select(1, false);
					lv->clear_selection();

					// ASSERT
					Assert::IsTrue(get_matching_indices(hlv, LVNI_SELECTED).empty());

					// ACT
					lv->select(2, false);
					lv->select(3, false);
					lv->clear_selection();

					// ASSERT
					Assert::IsTrue(get_matching_indices(hlv, LVNI_SELECTED).empty());
				}


				[TestMethod]
				void RequestProperTrackableOnFocusChange()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100, 1));
					NMLISTVIEW nmlv = {
						{	0, 0, LVN_ITEMCHANGED	},
						/* iItem = */ 0, /* iSubItem = */ 0,
					};

					lv->set_model(m);
					lv->add_column(L"iiii", listview::dir_none);
					lv->adjust_column_widths();

					// ACT
					nmlv.iItem = 2, nmlv.uOldState = 0, nmlv.uNewState = LVIS_FOCUSED;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));

					// ASSERT
					Assert::IsTrue(1 == m->tracking_requested.size());
					Assert::IsTrue(2 == m->tracking_requested[0]);

					// ACT
					nmlv.iItem = 3, nmlv.uOldState = 0, nmlv.uNewState = LVIS_FOCUSED;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));

					// ASSERT
					Assert::IsTrue(2 == m->tracking_requested.size());
					Assert::IsTrue(3 == m->tracking_requested[1]);

					// ACT
					nmlv.iItem = 5, nmlv.uOldState = LVIS_SELECTED, nmlv.uNewState = LVIS_SELECTED | LVIS_FOCUSED;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
					ListView_SetItemState(hlv, 7, LVIS_FOCUSED, LVIS_FOCUSED);

					// ASSERT
					Assert::IsTrue(4 == m->tracking_requested.size());
					Assert::IsTrue(5 == m->tracking_requested[2]);
					Assert::IsTrue(7 == m->tracking_requested[3]);
				}


				[TestMethod]
				void NotEnteringToFocusedDoesNotLeadToTracking()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100, 1));
					NMLISTVIEW nmlv = {
						{	0, 0, LVN_ITEMCHANGED	},
						/* iItem = */ 0, /* iSubItem = */ 0,
					};

					lv->set_model(m);
					lv->add_column(L"iiii", listview::dir_none);
					lv->adjust_column_widths();

					// ACT
					nmlv.iItem = 2, nmlv.uOldState = LVIS_FOCUSED, nmlv.uNewState = LVIS_FOCUSED;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
					nmlv.iItem = 3, nmlv.uOldState = LVIS_FOCUSED, nmlv.uNewState = 0;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
					nmlv.iItem = 2, nmlv.uOldState = LVIS_SELECTED | LVIS_FOCUSED, nmlv.uNewState = LVIS_SELECTED | LVIS_FOCUSED;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));
					nmlv.iItem = 3, nmlv.uOldState = LVIS_ACTIVATING | LVIS_FOCUSED, nmlv.uNewState = LVIS_FOCUSED;
					::SendMessage(hlv, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlv));

					// ASSERT
					Assert::IsTrue(m->tracking_requested.empty());
				}


				[TestMethod]
				void TakeOwnershipOverTrackable()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100));
					shared_ptr<const listview::trackable> t(new mock_trackable());
					weak_ptr<const listview::trackable> wt(t);
					vector<int> matched;

					lv->set_model(m);
					swap(m->trackables[5], t);

					// ACT
					ListView_SetItemState(hlv, 5, LVIS_FOCUSED, LVIS_FOCUSED);
					m->trackables.clear();
					t = trackable_ptr();

					// ASSERT
					Assert::IsFalse(wt.expired());
				}


				[TestMethod]
				void ReleaseTrackableOnRemoveFocus()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100));
					weak_ptr<const listview::trackable> wt(mock_trackable::add(m->trackables, 5));
					vector<int> matched;

					lv->set_model(m);
					ListView_SetItemState(hlv, 5, LVIS_FOCUSED, LVIS_FOCUSED);
					m->trackables.clear();

					// ACT
					ListView_SetItemState(hlv, 5, 0, LVIS_FOCUSED);

					// ASSERT
					Assert::IsTrue(wt.expired());
				}


				[TestMethod]
				void ReleaseTrackableOnNewFocus()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100));
					weak_ptr<const listview::trackable> wt(mock_trackable::add(m->trackables, 5));
					vector<int> matched;

					lv->set_model(m);
					ListView_SetItemState(hlv, 5, LVIS_FOCUSED, LVIS_FOCUSED);
					m->trackables.clear();

					// ACT
					ListView_SetItemState(hlv, 7, LVIS_FOCUSED, LVIS_FOCUSED);

					// ASSERT
					Assert::IsTrue(wt.expired());
				}


				[TestMethod]
				void ResetFocusOnInvalidation()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100));
					trackable_ptr t(mock_trackable::add(m->trackables, 5));
					vector<int> matched;

					lv->set_model(m);

					ListView_SetItemState(hlv, 5, LVIS_FOCUSED, LVIS_FOCUSED);
					m->tracking_requested.clear();

					// ACT
					t->track_result = 7;
					m->invalidated(100);

					// ASSERT
					matched = get_matching_indices(hlv, LVNI_FOCUSED);
					Assert::IsTrue(1 == matched.size());
					Assert::IsTrue(7 == matched[0]);
					Assert::IsTrue(m->tracking_requested.empty());

					// ACT
					t->track_result = 13;
					m->invalidated(100);

					// ASSERT
					matched = get_matching_indices(hlv, LVNI_FOCUSED);
					Assert::IsTrue(1 == matched.size());
					Assert::IsTrue(13 == matched[0]);
					Assert::IsTrue(m->tracking_requested.empty());
				}


				[TestMethod]
				void ReleaseTrackableOnBecameInvalid()
				{
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100));
					trackable_ptr t(mock_trackable::add(m->trackables, 5));
					weak_ptr<const listview::trackable> wt(t);
					vector<int> matched;

					lv->set_model(m);

					ListView_SetItemState(hlv, 5, LVIS_FOCUSED, LVIS_FOCUSED);
					m->trackables.clear();

					// ACT
					t->track_result = listview::npos;
					t.reset();
					m->invalidated(100);

					// ASSERT
					Assert::IsTrue(wt.expired());
					Assert::IsTrue(get_matching_indices(hlv, LVNI_FOCUSED).empty());
				}


				[TestMethod]
				void ResetSelectionOnInvalidation1()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100));
					trackable_ptr t[] = {
						mock_trackable::add(m->trackables, 5),
						mock_trackable::add(m->trackables, 7),
						mock_trackable::add(m->trackables, 17),
					};
					vector<int> matched;

					lv->set_model(m);

					ListView_SetItemState(hlv, 7, LVIS_SELECTED, LVIS_SELECTED);
					ListView_SetItemState(hlv, 5, LVIS_SELECTED, LVIS_SELECTED);	// order is disturbed intentionally!
					ListView_SetItemState(hlv, 17, LVIS_SELECTED, LVIS_SELECTED);
					m->tracking_requested.clear();

					// ACT
					t[2]->track_result = 21;
					m->invalidated(100);

					// ASSERT
					listview::index_type expected1[] = {	5, 7, 21,	};

					ut::AreEquivalent(expected1, get_matching_indices(hlv, LVNI_SELECTED));

					// ACT
					t[0]->track_result = 13;
					m->invalidated(100);

					// ASSERT
					listview::index_type expected2[] = {	13, 7, 21,	};

					ut::AreEquivalent(expected2, get_matching_indices(hlv, LVNI_SELECTED));

					// ACT
					t[1]->track_result = listview::npos;
					m->invalidated(100);

					// ASSERT
					listview::index_type expected3[] = {	13, 21,	};

					ut::AreEquivalent(expected3, get_matching_indices(hlv, LVNI_SELECTED));
					Assert::IsTrue(m->tracking_requested.empty());
				}


				[TestMethod]
				void ResetSelectionOnInvalidation2()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100));
					trackable_ptr t[] = {
						mock_trackable::add(m->trackables, 5),
						mock_trackable::add(m->trackables, 7),
						mock_trackable::add(m->trackables, 17),
						mock_trackable::add(m->trackables, 19),
						mock_trackable::add(m->trackables, 23),
					};
					vector<int> matched;

					lv->set_model(m);

					ListView_SetItemState(hlv, 19, LVIS_SELECTED, LVIS_SELECTED);
					ListView_SetItemState(hlv, 5, LVIS_SELECTED, LVIS_SELECTED);	// order is disturbed intentionally!
					ListView_SetItemState(hlv, 17, LVIS_SELECTED, LVIS_SELECTED);
					ListView_SetItemState(hlv, 7, LVIS_SELECTED, LVIS_SELECTED);
					ListView_SetItemState(hlv, 23, LVIS_SELECTED, LVIS_SELECTED);
					m->tracking_requested.clear();

					// ACT
					t[2]->track_result = 3;
					t[4]->track_result = 47;
					m->set_count(100);

					// ASSERT
					listview::index_type expected1[] = {	5, 7, 3, 19, 47,	};

					ut::AreEquivalent(expected1, get_matching_indices(hlv, LVNI_SELECTED));

					// ACT
					t[3]->track_result = 1;
					m->set_count(100);

					// ASSERT
					listview::index_type expected2[] = {	5, 7, 3, 1, 47,	};

					ut::AreEquivalent(expected2, get_matching_indices(hlv, LVNI_SELECTED));
					Assert::IsTrue(m->tracking_requested.empty());
				}


				[TestMethod]
				void ReleaseLostTrackables()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100));
					trackable_ptr t(mock_trackable::add(m->trackables, 4));
					weak_ptr<const listview::trackable> wt[] = {
						t,
						mock_trackable::add(m->trackables, 8),
						mock_trackable::add(m->trackables, 16),
					};

					lv->set_model(m);
					ListView_SetItemState(hlv, 4, LVIS_SELECTED, LVIS_SELECTED);
					ListView_SetItemState(hlv, 8, LVIS_SELECTED, LVIS_SELECTED);
					ListView_SetItemState(hlv, 16, LVIS_SELECTED, LVIS_SELECTED);
					m->trackables.clear();

					// ACT
					t->track_result = listview::npos;
					t.reset();
					m->set_count(99);

					// ASSERT
					Assert::IsTrue(wt[0].expired());
					Assert::IsFalse(wt[1].expired());
					Assert::IsFalse(wt[2].expired());
				}


				[TestMethod]
				void ReleaseTrackablesForDeselectedItems()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100));
					weak_ptr<const listview::trackable> wt[] = {
						mock_trackable::add(m->trackables, 4),
						mock_trackable::add(m->trackables, 8),
						mock_trackable::add(m->trackables, 16),
					};

					lv->set_model(m);
					ListView_SetItemState(hlv, 4, LVIS_SELECTED, LVIS_SELECTED);
					ListView_SetItemState(hlv, 8, LVIS_SELECTED, LVIS_SELECTED);
					ListView_SetItemState(hlv, 16, LVIS_SELECTED, LVIS_SELECTED);
					m->trackables.clear();

					// ACT
					ListView_SetItemState(hlv, 4, 0, LVIS_SELECTED);
					ListView_SetItemState(hlv, 16, 0, LVIS_SELECTED);

					// ASSERT
					Assert::IsTrue(wt[0].expired());
					Assert::IsFalse(wt[1].expired());
					Assert::IsTrue(wt[2].expired());
				}


				[TestMethod]
				void AbandonDeselectedItemTracking()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100));
					trackable_ptr t[] = {
						mock_trackable::add(m->trackables, 5),
						mock_trackable::add(m->trackables, 7),
						mock_trackable::add(m->trackables, 17),
					};

					lv->set_model(m);

					ListView_SetItemState(hlv, 7, LVIS_SELECTED, LVIS_SELECTED);
					ListView_SetItemState(hlv, 5, LVIS_SELECTED, LVIS_SELECTED);
					ListView_SetItemState(hlv, 17, LVIS_SELECTED, LVIS_SELECTED);

					// ACT
					ListView_SetItemState(hlv, 17, 0, LVIS_SELECTED);
					t[0]->track_result = 3;
					t[2]->track_result = 21;
					m->set_count(100);

					// ASSERT
					listview::index_type expected1[] = {	3, 7,	};

					ut::AreEquivalent(expected1, get_matching_indices(hlv, LVNI_SELECTED));

					// ACT
					ListView_SetItemState(hlv, 3, 0, LVIS_SELECTED);
					t[0]->track_result = 13;
					m->set_count(100);

					// ASSERT
					listview::index_type expected2[] = {	7,	};

					ut::AreEquivalent(expected2, get_matching_indices(hlv, LVNI_SELECTED));
				}


				[TestMethod]
				void AbandonTrackingOnDeselectAllItems()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100));
					trackable_ptr t[] = {
						mock_trackable::add(m->trackables, 5),
						mock_trackable::add(m->trackables, 7),
						mock_trackable::add(m->trackables, 17),
					};

					lv->set_model(m);

					ListView_SetItemState(hlv, 7, LVIS_SELECTED, LVIS_SELECTED);
					ListView_SetItemState(hlv, 5, LVIS_SELECTED, LVIS_SELECTED);
					ListView_SetItemState(hlv, 17, LVIS_SELECTED, LVIS_SELECTED);
					m->tracking_requested.clear();

					// ACT
					ListView_SetItemState(hlv, -1, 0, LVIS_SELECTED);
					t[0]->track_result = 3;
					t[0]->track_result = 4;
					t[2]->track_result = 21;
					m->set_count(100);

					// ASSERT
					Assert::IsTrue(get_matching_indices(hlv, LVNI_SELECTED).empty());
					Assert::IsTrue(m->tracking_requested.empty());
				}


				[TestMethod]
				void FocusAndSelectionRequestTrackablesTwice()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100));
					trackable_ptr t(mock_trackable::add(m->trackables, 7));
					
					lv->set_model(m);
					m->tracking_requested.clear();

					// ACT
					ListView_SetItemState(hlv, 7, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

					// ASSERT
					Assert::IsTrue(2 == m->tracking_requested.size());
					Assert::IsTrue(7 == m->tracking_requested[0]);
					Assert::IsTrue(7 == m->tracking_requested[1]);
				}


				[TestMethod]
				void IgnoreNullTrackableForSelected()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100));
					
					lv->set_model(m);
					ListView_SetItemState(hlv, 7, LVIS_SELECTED, LVIS_SELECTED);
					ListView_SetItemState(hlv, 8, LVIS_SELECTED, LVIS_SELECTED);

					// ACT / ASSERT (must not throw)
					m->set_count(100);

					// ASSERT
					listview::index_type expected[] = {	7, 8,	};

					ut::AreEquivalent(expected, get_matching_indices(hlv, LVNI_SELECTED));
				}


				[TestMethod]
				void ReleaseTrackablesOnModelChange()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100));
					weak_ptr<const listview::trackable> wt[] = {
						mock_trackable::add(m->trackables, 4),
						mock_trackable::add(m->trackables, 8),
						mock_trackable::add(m->trackables, 16),
					};

					lv->set_model(m);
					ListView_SetItemState(hlv, 4, LVIS_SELECTED, LVIS_SELECTED);
					ListView_SetItemState(hlv, 8, LVIS_SELECTED, LVIS_SELECTED);
					ListView_SetItemState(hlv, 16, LVIS_FOCUSED, LVIS_FOCUSED);
					m->trackables.clear();

					// ACT
					lv->set_model(model_ptr(new mock_model(130)));

					// ASSERT
					Assert::IsTrue(wt[0].expired());
					Assert::IsTrue(wt[1].expired());
					Assert::IsTrue(wt[2].expired());
				}


				[TestMethod]
				void EnsureItemVisibility()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));

					lv->set_model(model_ptr(new mock_model(100, 1)));
					lv->add_column(L"iiii", listview::dir_none);
					lv->adjust_column_widths();

					// ACT
					lv->ensure_visible(99);

					// ASSERT
					Assert::IsTrue(is_item_visible(hlv, 99));
					Assert::IsFalse(is_item_visible(hlv, 49));
					Assert::IsFalse(is_item_visible(hlv, 0));

					// ACT
					lv->ensure_visible(49);

					// ASSERT
					Assert::IsFalse(is_item_visible(hlv, 99));
					Assert::IsTrue(is_item_visible(hlv, 49));
					Assert::IsFalse(is_item_visible(hlv, 0));

					// ACT
					lv->ensure_visible(0);

					// ASSERT
					Assert::IsFalse(is_item_visible(hlv, 99));
					Assert::IsFalse(is_item_visible(hlv, 49));
					Assert::IsTrue(is_item_visible(hlv, 0));
				}


				[TestMethod]
				void CaptureTrackableOnSettingVisibilityTracking()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100, 1));

					lv->add_column(L"iiii", listview::dir_none);
					lv->adjust_column_widths();
					lv->set_model(m);

					// ACT
					lv->ensure_visible(0);

					// ASSERT
					Assert::IsTrue(1 == m->tracking_requested.size());
					Assert::IsTrue(0 == m->tracking_requested[0]);

					// ACT
					lv->ensure_visible(1);
					lv->ensure_visible(7);

					// ASSERT
					Assert::IsTrue(3 == m->tracking_requested.size());
					Assert::IsTrue(1 == m->tracking_requested[1]);
					Assert::IsTrue(7 == m->tracking_requested[2]);
				}


				[TestMethod]
				void ReleaseTrackableOnChangingVisibilityTracking()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100, 1));
					weak_ptr<const listview::trackable> wt[] = {
						mock_trackable::add(m->trackables, 4),
						mock_trackable::add(m->trackables, 7),
					};

					lv->add_column(L"iiii", listview::dir_none);
					lv->adjust_column_widths();
					lv->set_model(m);

					// ACT
					lv->ensure_visible(4);
					lv->ensure_visible(7);
					m->trackables.clear();

					// ASSERT
					Assert::IsTrue(wt[0].expired());
					Assert::IsFalse(wt[1].expired());
				}


				[TestMethod]
				void ReleaseVisibilityTrackableOnChangingModel()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m1(new mock_model(100, 1)), m2(new mock_model(100, 1));
					weak_ptr<const listview::trackable> wt = mock_trackable::add(m1->trackables, 5);

					lv->add_column(L"iiii", listview::dir_none);
					lv->adjust_column_widths();
					lv->set_model(m1);
					lv->ensure_visible(5);
					m1->trackables.clear();

					// ACT
					lv->set_model(m2);

					// ASSERT
					Assert::IsTrue(wt.expired());
				}


				[TestMethod]
				void EnsureItemVisibilityInDynamics()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100, 1));
					trackable_ptr t(mock_trackable::add(m->trackables, 0));

					lv->set_model(m);
					lv->add_column(L"iiii", listview::dir_none);
					lv->adjust_column_widths();
					lv->ensure_visible(0);

					// ACT
					t->track_result = 99;
					m->set_count(100);

					// ASSERT
					Assert::IsTrue(is_item_visible(hlv, 99));
					Assert::IsFalse(is_item_visible(hlv, 49));
					Assert::IsFalse(is_item_visible(hlv, 0));

					// ACT
					t->track_result = 49;
					m->set_count(100);

					// ASSERT
					Assert::IsFalse(is_item_visible(hlv, 99));
					Assert::IsTrue(is_item_visible(hlv, 49));
					Assert::IsFalse(is_item_visible(hlv, 0));

					// ACT
					t->track_result = 0;
					m->set_count(100);

					// ASSERT
					Assert::IsFalse(is_item_visible(hlv, 99));
					Assert::IsFalse(is_item_visible(hlv, 49));
					Assert::IsTrue(is_item_visible(hlv, 0));
				}


				[TestMethod]
				void DontTrackItemIfItWasHidden1()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100, 1));
					trackable_ptr t(mock_trackable::add(m->trackables, 0));

					lv->set_model(m);
					lv->add_column(L"iiii", listview::dir_none);
					lv->adjust_column_widths();
					lv->ensure_visible(0);

					// ACT
					ListView_EnsureVisible(hlv, 49, FALSE);
					t->track_result = 99;
					m->set_count(100);

					// ASSERT
					Assert::IsFalse(is_item_visible(hlv, 0));
					Assert::IsTrue(is_item_visible(hlv, 49));
					Assert::IsFalse(is_item_visible(hlv, 99));

					// ACT
					ListView_EnsureVisible(hlv, 0, FALSE);
					t->track_result = 49;
					m->set_count(100);

					// ASSERT
					Assert::IsTrue(is_item_visible(hlv, 0));
					Assert::IsFalse(is_item_visible(hlv, 49));
					Assert::IsFalse(is_item_visible(hlv, 99));
				}


				[TestMethod]
				void DontTrackItemIfItWasHidden2()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100, 1));
					trackable_ptr t(mock_trackable::add(m->trackables, 49));

					lv->set_model(m);
					lv->add_column(L"iiii", listview::dir_none);
					lv->adjust_column_widths();
					lv->ensure_visible(49);

					// ACT
					ListView_EnsureVisible(hlv, 0, FALSE);
					t->track_result = 99;
					m->set_count(100);

					// ASSERT
					Assert::IsTrue(is_item_visible(hlv, 0));
					Assert::IsFalse(is_item_visible(hlv, 49));
					Assert::IsFalse(is_item_visible(hlv, 99));
				}


				[TestMethod]
				void RestoreVisibilityTracking()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(100, 1));
					trackable_ptr t(mock_trackable::add(m->trackables, 49));

					lv->set_model(m);
					lv->add_column(L"iiii", listview::dir_none);
					lv->adjust_column_widths();
					lv->ensure_visible(49);
					ListView_EnsureVisible(hlv, 0, FALSE);
					t->track_result = 99;
					m->set_count(100);

					// ACT
					ListView_EnsureVisible(hlv, 99, FALSE);
					t->track_result = 49;
					m->set_count(100);

					// ASSERT
					Assert::IsFalse(is_item_visible(hlv, 0));
					Assert::IsTrue(is_item_visible(hlv, 49));
					Assert::IsFalse(is_item_visible(hlv, 99));
				}


				[TestMethod]
				void FirstVisibleItemIsNotObscuredByHeader()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(10, 1));
					trackable_ptr t(mock_trackable::add(m->trackables, 9));

					lv->add_column(L"iiii", listview::dir_none);
					lv->adjust_column_widths();
					lv->set_model(m);

					::SetWindowLong(hlv, GWL_STYLE, ::GetWindowLong(hlv, GWL_STYLE) | LVS_NOCOLUMNHEADER);
					DWORD dwsize = ListView_ApproximateViewRect(hlv, -1, -1, -1);
					::SetWindowLong(hlv, GWL_STYLE, ::GetWindowLong(hlv, GWL_STYLE) & ~LVS_NOCOLUMNHEADER);

					::MoveWindow(::GetParent(hlv), 0, 0, LOWORD(dwsize), HIWORD(dwsize), TRUE);
					::MoveWindow(hlv, 0, 0, LOWORD(dwsize), HIWORD(dwsize), TRUE);

					// ACT
					lv->ensure_visible(9);
					int first_visible = ListView_GetTopIndex(hlv);

					// ASSERT
					Assert::IsFalse(0 == first_visible);

					// ACT
					t->track_result = 0;
					m->set_count(10);
					first_visible = ListView_GetTopIndex(hlv);

					// ASSERT
					Assert::IsTrue(0 == first_visible);
				}


				[TestMethod]
				void ListViewIsNotScrolledIfNoHeaderAndViewRectMatchesNumberOfItems()
				{
					// INIT
					HWND hlv = create_listview();
					shared_ptr<listview> lv(wrap_listview(hlv));
					model_ptr m(new mock_model(10, 1));
					trackable_ptr t(mock_trackable::add(m->trackables, 9));

					lv->add_column(L"iiii", listview::dir_none);
					lv->adjust_column_widths();
					lv->set_model(m);

					::SetWindowLong(hlv, GWL_STYLE, ::GetWindowLong(hlv, GWL_STYLE) | LVS_NOCOLUMNHEADER);
					DWORD dwsize = ListView_ApproximateViewRect(hlv, -1, -1, -1);

					::MoveWindow(::GetParent(hlv), 0, 0, LOWORD(dwsize), HIWORD(dwsize), TRUE);
					::MoveWindow(hlv, 0, 0, LOWORD(dwsize), HIWORD(dwsize), TRUE);

					// ACT
					lv->ensure_visible(9);
					int first_visible = ListView_GetTopIndex(hlv);

					// ASSERT
					Assert::IsTrue(0 == first_visible);

					// ACT
					t->track_result = 0;
					m->set_count(10);
					first_visible = ListView_GetTopIndex(hlv);

					// ASSERT
					Assert::IsTrue(0 == first_visible);
				}
			};
		}
	}
}
