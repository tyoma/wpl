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

				static HWND get_window_and_resize(HWND hparent, visual &v, int cx, int cy)
				{
					visual::positioned_native_views nviews;
					v.resize(cx, cy, nviews);
					assert_is_true(1u <= nviews.size());
					return nviews[0].get_view().get_window(hparent);
				}

				template <typename T>
				void push_back(vector<T> &container, const T &value)
				{	container.push_back(value);	}

				template <typename T>
				void reset(shared_ptr<T> &ptr)
				{	ptr.reset();	}
			}

			begin_test_suite( ButtonTests )

				WindowManager wmanager;
				HWND parent;

				init( CreateParent )
				{
					parent = wmanager.create_visible_window();
				}


				test( ButtonControlIsANativeView )
				{
					// INIT
					shared_ptr<button> b = create_button();
					window_tracker wt;

					// ACT
					HWND hwnd = get_window_and_resize(parent, *b, 100, 20);

					// ASSERT
					wt.checkpoint();

					assert_equal(wt.find_created(WC_BUTTON)[0], hwnd);
					assert_equal(parent, ::GetParent(hwnd));

					// ACT / ASSERT (happens in get_window_and_resize)
					assert_equal(hwnd, get_window_and_resize(parent, *b, 19, 7));

					// ASSERT
					wt.checkpoint();

					assert_equal(1u, wt.created.size());
				}


				test( ClickingAButtonRaisesClickedSignal )
				{
					// INIT
					int clicks = 0;
					shared_ptr<button> b = create_button();
					HWND hbutton = get_window_and_resize(parent, *b, 100, 100);
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
					HWND hbutton = get_window_and_resize(parent, *b, 100, 20);

					// ACT
					b->set_text(L"Launch!");

					// ASSERT
					assert_equal(L"Launch!", get_window_text(hbutton));

					// ACT
					b->set_text(L"Let's go...");

					// ASSERT
					assert_equal(L"Let's go...", get_window_text(hbutton));
				}


				test( TextSetIsPreservedBeforeTheMaterialization )
				{
					// INIT
					shared_ptr<button> b[] = { create_button(), create_button(), };

					// ACT
					b[0]->set_text(L"Launch!");
					b[1]->set_text(L"Stop...");
					HWND hbutton1 = get_window_and_resize(parent, *b[0], 100, 20);
					HWND hbutton2 = get_window_and_resize(parent, *b[1], 100, 20);

					// ASSERT
					assert_equal(L"Launch!", get_window_text(hbutton1));
					assert_equal(L"Stop...", get_window_text(hbutton2));
				}


				test( SelfDestructIsOK )
				{
					// INIT
					shared_ptr<button> b = create_button();
					HWND hbutton = get_window_and_resize(parent, *b, 100, 20);
					slot_connection c = b->clicked += bind(&reset<button>, ref(b));

					// ACT
					::SendMessage(hbutton, OCM_COMMAND, BN_CLICKED << 16, 0);

					// ASSERT
					assert_null(b);
				}
			end_test_suite


			begin_test_suite( LinkTests )

				WindowManager wmanager;
				HWND parent;

				init( CreateParent )
				{
					parent = wmanager.create_visible_window();
				}


				test( LinkControlIsANativeView )
				{
					// INIT
					shared_ptr<link> b = create_link();
					window_tracker wt;

					// ACT
					HWND hwnd = get_window_and_resize(parent, *b, 100, 20);

					// ASSERT
					wt.checkpoint();

					assert_equal(wt.find_created(L"SysLink")[0], hwnd);
					assert_equal(parent, ::GetParent(hwnd));

					// ACT / ASSERT (happens in get_window_and_resize)
					assert_equal(hwnd, get_window_and_resize(parent, *b, 19, 7));

					// ASSERT
					wt.checkpoint();

					assert_equal(1u, wt.created.size());
				}


				test( LinkControlAlignmentCanBeChangedBeforeTheWindowIsCreated )
				{
					// INIT
					shared_ptr<link> b = create_link();

					// ACT
					b->set_align(text_container::left);
					HWND hlink = get_window_and_resize(parent, *b, 100, 20);

					// ASSERT
					assert_equal(0, LWS_RIGHT & ::GetWindowLong(hlink, GWL_STYLE));

					// ACT
					b = create_link();
					b->set_align(text_container::right);
					hlink = get_window_and_resize(parent, *b, 100, 20);

					// ASSERT
					assert_equal(LWS_RIGHT, LWS_RIGHT & ::GetWindowLong(hlink, GWL_STYLE));
				}


				test( LinkControlAlignmentCanBeChanged )
				{
					// INIT
					shared_ptr<link> b = create_link();
					window_tracker wt;

					// ACT
					HWND hlink = get_window_and_resize(parent, *b, 100, 20);

					// ASSERT
					assert_equal(0, LWS_RIGHT & ::GetWindowLong(hlink, GWL_STYLE));
					
					// ACT
					b->set_align(text_container::center);

					// ASSERT
					assert_equal(0, LWS_RIGHT & ::GetWindowLong(hlink, GWL_STYLE));

					// ACT
					b->set_align(text_container::right);

					// ASSERT
					assert_equal(LWS_RIGHT, LWS_RIGHT & ::GetWindowLong(hlink, GWL_STYLE));

					// ACT
					b->set_align(text_container::center);

					// ASSERT
					assert_equal(LWS_RIGHT, LWS_RIGHT & ::GetWindowLong(hlink, GWL_STYLE));

					// ACT
					b->set_align(text_container::left);

					// ASSERT
					assert_equal(0, LWS_RIGHT & ::GetWindowLong(hlink, GWL_STYLE));
				}


				test( ClickingALinkRaisesClickedSignal )
				{
					// INIT
					vector<size_t> log_ids;
					vector<wstring> log_links;
					shared_ptr<link> l = create_link();
					HWND hlink = get_window_and_resize(parent, *l, 100, 20);
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
					HWND hlink = get_window_and_resize(parent, *l, 100, 20);

					// ACT
					l->set_text(L"Launch!");

					// ASSERT
					assert_equal(L"Launch!", get_window_text(hlink));

					// ACT
					l->set_text(L"Let's <a href=\"zz\">go</a>...");

					// ASSERT
					assert_equal(L"Let's <a href=\"zz\">go</a>...", get_window_text(hlink));
				}


				test( TextSetIsPreservedBeforeTheMaterialization )
				{
					// INIT
					shared_ptr<link> l[] = { create_link(), create_link(), };

					// ACT
					l[0]->set_text(L"Launch <a>the rocket</a>!");
					l[1]->set_text(L"Stop...");
					HWND hlink1 = get_window_and_resize(parent, *l[0], 100, 20);
					HWND hlink2 = get_window_and_resize(parent, *l[1], 100, 20);

					// ASSERT
					assert_equal(L"Launch <a>the rocket</a>!", get_window_text(hlink1));
					assert_equal(L"Stop...", get_window_text(hlink2));
				}


				test( SelfDestructIsOK )
				{
					// INIT
					shared_ptr<link> l = create_link();
					HWND hlink = get_window_and_resize(parent, *l, 100, 20);
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
