#include <wpl/ui/controls.h>

#include <wpl/ui/win32/controls.h>
#include <wpl/ui/win32/native_view.h>

#include "TestHelpers.h"

#include <functional>
#include <windows.h>
#include <commctrl.h>
#include <olectl.h>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;
using namespace placeholders;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace
			{
				void increment(int *value)
				{	++*value;	}

				static HWND get_window_and_resize(visual &v, int cx, int cy)
				{
					visual::positioned_native_views nviews;
					v.resize(cx, cy, nviews);
					assert_is_true(1u <= nviews.size());
					return nviews[0].get_view().get_window();
				}

				template <typename T>
				void push_back(vector<T> &container, const T &value)
				{	container.push_back(value);	}

				template <typename T>
				void reset(shared_ptr<T> &ptr)
				{	ptr.reset();	}
			}

			begin_test_suite( ButtonTests )

				test( ButtonCreatesNativeButton )
				{
					// INIT
					window_tracker wt;

					// INIT / ACT
					shared_ptr<button> b = create_button();

					// ASSERT
					wt.checkpoint();

					assert_not_null(b);
					assert_equal(1u, wt.find_created(WC_BUTTON).size());
				}


				test( ButtonControlIsNativeView )
				{
					// INIT
					window_tracker wt;
					shared_ptr<button> b = create_button();

					// ACT
					HWND hwnd = get_window_and_resize(*b, 100, 20);

					// ASSERT
					wt.checkpoint();

					assert_equal(wt.find_created(WC_BUTTON)[0], hwnd);

					// ACT / ASSERT (happens in get_window_and_resize)
					get_window_and_resize(*b, 19, 7);
				}


				test( ClickingAButtonRaisesClickedSignal )
				{
					// INIT
					int clicks = 0;
					shared_ptr<button> b = create_button();
					HWND hbutton = get_window_and_resize(*b, 100, 100);
					slot_connection c = b->clicked += bind(&increment, &clicks);
										
					// ACT
					::SendMessage(hbutton, OCM_COMMAND, BN_CLICKED << 16, 0);

					// ASSERT
					assert_equal(1, clicks);

					// ACT
					::SendMessage(hbutton, OCM_COMMAND, BN_CLICKED << 16, 0);

					// ASSERT
					assert_equal(2, clicks);
				}


				test( SettingTextChangesButtonText )
				{
					// INIT
					shared_ptr<button> b = create_button();
					HWND hbutton = get_window_and_resize(*b, 100, 20);

					// ACT
					b->set_text(L"Launch!");

					// ASSERT
					assert_equal(L"Launch!", get_window_text(hbutton));

					// ACT
					b->set_text(L"Let's go...");

					// ASSERT
					assert_equal(L"Let's go...", get_window_text(hbutton));
				}


				test( SelfDestructIsOK )
				{
					// INIT
					shared_ptr<button> b = create_button();
					HWND hbutton = get_window_and_resize(*b, 100, 20);
					slot_connection c = b->clicked += bind(&reset<button>, ref(b));

					// ACT
					::SendMessage(hbutton, OCM_COMMAND, BN_CLICKED << 16, 0);

					// ASSERT
					assert_null(b);
				}
			end_test_suite


			begin_test_suite( LinkTests )

				test( LinkCreatesNativeLink )
				{
					// INIT
					window_tracker wt;

					// INIT / ACT
					shared_ptr<link> l = create_link();

					// ASSERT
					wt.checkpoint();

					assert_not_null(l);
					assert_equal(1u, wt.find_created(WC_LINK).size());
				}


				test( ButtonControlIsNativeView )
				{
					// INIT
					window_tracker wt;
					shared_ptr<button> b = create_button();

					// ACT
					HWND hwnd = get_window_and_resize(*b, 100, 20);

					// ASSERT
					wt.checkpoint();

					assert_equal(wt.find_created(WC_BUTTON)[0], hwnd);

					// ACT / ASSERT (happens in get_window_and_resize)
					get_window_and_resize(*b, 19, 7);
				}


				test( ClickingALinkRaisesClickedSignal )
				{
					// INIT
					vector<size_t> log_ids;
					vector<wstring> log_links;
					shared_ptr<link> l = create_link();
					HWND hlink = get_window_and_resize(*l, 100, 20);
					slot_connection c1 = l->clicked += std::bind(&push_back<size_t>, ref(log_ids), _1);
					slot_connection c2 = l->clicked += std::bind(&push_back<wstring>, ref(log_links), _2);
					NMLINK nmlink = {};

					nmlink.hdr.code = NM_CLICK;
					nmlink.item.iLink = 0;
					wcscpy(nmlink.item.szUrl, L"Test #1");
										
					// ACT
					::SendMessage(hlink, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlink));

					// ASSERT
					size_t reference_ids1[] = { 0u, };
					wstring reference_links1[] = { L"Test #1", };

					assert_equal(reference_ids1, log_ids);
					assert_equal(reference_links1, log_links);

					// INIT
					nmlink.item.iLink = 3;
					wcscpy(nmlink.item.szUrl, L"https://github.com");

					// ACT
					::SendMessage(hlink, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlink));

					// ASSERT
					size_t reference_ids2[] = { 0u, 3u, };
					wstring reference_links2[] = { L"Test #1", L"https://github.com", };

					assert_equal(reference_ids2, log_ids);
					assert_equal(reference_links2, log_links);
				}


				test( SettingTextChangesButtonText )
				{
					// INIT
					shared_ptr<link> l = create_link();
					HWND hlink = get_window_and_resize(*l, 100, 20);

					// ACT
					l->set_text(L"Launch!");

					// ASSERT
					assert_equal(L"Launch!", get_window_text(hlink));

					// ACT
					l->set_text(L"Let's <a href=\"zz\">go</a>...");

					// ASSERT
					assert_equal(L"Let's <a href=\"zz\">go</a>...", get_window_text(hlink));
				}


				test( SelfDestructIsOK )
				{
					// INIT
					shared_ptr<link> l = create_link();
					HWND hlink = get_window_and_resize(*l, 100, 20);
					slot_connection c = l->clicked += bind(&reset<link>, ref(l));
					NMLINK nmlink = {};

					nmlink.hdr.code = NM_CLICK;
					nmlink.item.iLink = 0;
										
					// ACT
					::SendMessage(hlink, OCM_NOTIFY, 0, reinterpret_cast<LPARAM>(&nmlink));

					// ASSERT
					assert_null(l);
				}
			end_test_suite
		}
	}
}
