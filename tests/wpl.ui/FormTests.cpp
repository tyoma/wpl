#include <wpl/ui/form.h>
#include <wpl/ui/win32/native_view.h>
#include <wpl/ui/win32/form.h>

#include "Mockups.h"
#include "MockupsNative.h"
#include "TestHelpers.h"

#include <tchar.h>
#include <ut/assert.h>
#include <ut/test.h>
#include <windows.h>

using namespace std;
using namespace std::placeholders;

namespace agge
{
	template <typename PixelT, typename RawBitmapT>
	bool operator ==(const bitmap<PixelT, RawBitmapT> &lhs, const bitmap<PixelT, RawBitmapT> &rhs)
	{
		if (lhs.width() != rhs.width() || lhs.height() != rhs.height())
			return false;
		for (count_t y = 0; y != lhs.height(); ++y)
		{
			for (count_t x = 0; x != lhs.width(); ++x)
			{
				if (memcmp(lhs.row_ptr(y) + x, rhs.row_ptr(y) + x, sizeof(pixel32)))
					return false;
			}
		}
		return true;
	}
}

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			namespace
			{
				typedef pair<shared_ptr<form>, HWND> form_and_handle;

				form_and_handle create_form_with_handle(HWND howner = 0)
				{
					window_tracker wt(L"#32770");
					shared_ptr<form> f(create_form(howner));

					wt.checkpoint();

					if (wt.created.size() != 1)
						throw runtime_error("Unexpected amount of windows created!");
					return make_pair(f, wt.created[0]);
				}

				shared_ptr<void> create_window(HWND hparent)
				{
					return shared_ptr<void>(::CreateWindow(_T("static"), NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hparent,
						NULL, NULL, 0), &::DestroyWindow);
				}

				void increment(int *counter)
				{	++*counter;	}

				shared_ptr<gcontext::surface_type> get_icon(HWND hwnd, bool big)
				{
					ICONINFO ii = {};
					HICON hicon = (HICON)::SendMessage(hwnd, WM_GETICON, big ? ICON_BIG : ICON_SMALL, 0);
					shared_ptr<void> hdc(::CreateCompatibleDC(NULL), &::DeleteDC);
					BITMAP b = {};

					::GetIconInfo(hicon, &ii);
					::GetObject(ii.hbmColor, sizeof(b), &b);

					assert_equal(32, b.bmBitsPixel);

					unique_ptr<byte[]> buffer(new byte[b.bmHeight * b.bmWidthBytes]);
					BITMAPINFO bi = {};

					bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
					::GetDIBits(static_cast<HDC>(hdc.get()), ii.hbmColor, 0, b.bmHeight, NULL, &bi, DIB_RGB_COLORS);
					bi.bmiHeader.biHeight = -bi.bmiHeader.biHeight;
					::GetDIBits(static_cast<HDC>(hdc.get()), ii.hbmColor, 0, b.bmHeight, buffer.get(), &bi, DIB_RGB_COLORS);

					shared_ptr<gcontext::surface_type> s(new gcontext::surface_type(b.bmWidth, b.bmHeight, 0));
					memcpy(s->row_ptr(0), buffer.get(), b.bmHeight * b.bmWidthBytes);
					return s;
				}
			}

			begin_test_suite( FormTests )
				WindowManager windowManager;

				init( Init )
				{
					windowManager.create_window();
				}

				teardown( Cleanup )
				{
					windowManager.Cleanup();
				}


				test( FormWindowIsCreatedAtFormConstruction )
				{
					// INIT
					window_tracker wt(L"#32770");

					// ACT
					shared_ptr<form> f1 = create_form();

					// ASSERT
					wt.checkpoint();

					assert_equal(1u, wt.created.size());
					assert_is_empty(wt.destroyed);

					// ACT
					shared_ptr<form> f2 = create_form();
					shared_ptr<form> f3 = create_form();

					// ASSERT
					wt.checkpoint();

					assert_equal(3u, wt.created.size());
					assert_is_empty(wt.destroyed);
				}


				test( FormConstructionReturnsNonNullObject )
				{
					// INIT / ACT / ASSERT
					assert_not_null(create_form());
				}


				test( FormDestructionDestroysItsWindow )
				{
					// INIT
					shared_ptr<form> f1 = create_form();
					shared_ptr<form> f2 = create_form();
					window_tracker wt(L"#32770");

					// ACT
					f1 = shared_ptr<form>();

					// ASSERT
					wt.checkpoint();

					assert_is_empty(wt.created);
					assert_equal(1u, wt.destroyed.size());

					// ACT
					f2 = shared_ptr<form>();

					// ASSERT
					wt.checkpoint();

					assert_is_empty(wt.created);
					assert_equal(2u, wt.destroyed.size());
				}


				test( FormWindowHasPopupStyleAndInvisibleAtConstruction )
				{
					// INIT / ACT
					form_and_handle f(create_form_with_handle());

					// ASSERT
					assert_is_true(has_style(f.second, WS_CAPTION | WS_THICKFRAME | WS_CLIPCHILDREN));
					assert_is_true(has_no_style(f.second, WS_VISIBLE));
				}


				test( ChangingFormVisibilityAffectsItsWindowVisibility )
				{
					// INIT
					form_and_handle f(create_form_with_handle());

					// ACT
					f.first->set_visible(true);

					// ASSERT
					assert_is_true(has_style(f.second, WS_VISIBLE));

					// ACT
					f.first->set_visible(false);

					// ASSERT
					assert_is_true(has_no_style(f.second, WS_VISIBLE));
				}


				test( SettingCaptionUpdatesWindowText )
				{
					// INIT
					form_and_handle f(create_form_with_handle());

					// ACT
					f.first->set_caption(L"Dialog #1...");

					// ASSERT
					assert_equal(L"Dialog #1...", get_window_text(f.second));

					// ACT
					f.first->set_caption(L"Are you sure?");

					// ASSERT
					assert_equal(L"Are you sure?", get_window_text(f.second));
				}


				test( ClosingWindowRaisesCloseSignal )
				{
					// INIT
					form_and_handle f(create_form_with_handle());
					int close_count = 0;
					slot_connection c = f.first->close += bind(&increment, &close_count);

					// ACT
					::SendMessage(f.second, WM_CLOSE, 0, 0);

					// ASSERT
					assert_equal(1, close_count);

					// ACT (the form still exists)
					::SendMessage(f.second, WM_CLOSE, 0, 0);

					// ASSERT
					assert_equal(2, close_count);
				}


				test( ResizingFormWindowLeadsToContentResize )
				{
					// INIT
					shared_ptr<mocks::logging_visual<view>> v(new mocks::logging_visual<view>());
					form_and_handle f(create_form_with_handle());
					RECT rc;

					f.first->set_view(v);
					v->resize_log.clear();

					// ACT
					::MoveWindow(f.second, 0, 0, 117, 213, TRUE);

					// ASSERT
					::GetClientRect(f.second, &rc);

					assert_equal(1u, v->resize_log.size());
					assert_equal(make_pair((int)rc.right, (int)rc.bottom), v->resize_log[0]);

					// ACT
					::MoveWindow(f.second, 27, 190, 531, 97, TRUE);

					// ASSERT
					::GetClientRect(f.second, &rc);

					assert_equal(2u, v->resize_log.size());
					assert_equal(make_pair((int)rc.right, (int)rc.bottom), v->resize_log[1]);
				}


				test( FormIsOwnedIfOwnerIsSpecified )
				{
					// INIT
					form_and_handle owner1(create_form_with_handle()), owner2(create_form_with_handle());

					// ACT
					form_and_handle owned1(create_form_with_handle(owner1.second));
					form_and_handle owned2(create_form_with_handle(owner2.second));

					// ASSERT
					assert_is_true(has_no_style(owned1.second, WS_CHILD));
					assert_equal(owner1.second, ::GetWindow(owned1.second, GW_OWNER));
					assert_is_true(has_no_style(owned2.second, WS_CHILD));
					assert_equal(owner2.second, ::GetWindow(owned2.second, GW_OWNER));
				}


				test( SettingTaskIconMakesItAvailableToTheSystem )
				{
					// INIT
					form_and_handle f1(create_form_with_handle()), f2(create_form_with_handle());
					gcontext::surface_type s1(30, 30, 0), s2(32, 32, 0);

					(s1.row_ptr(1) + 2)->components[0] = 0x12;
					(s1.row_ptr(3) + 7)->components[1] = 0x24;
					(s2.row_ptr(10) + 20)->components[3] = 0x71;

					// ACT
					f1.first->set_task_icon(s1);
					f2.first->set_task_icon(s2);

					// ASSERT
					assert_equal(s1, *get_icon(f1.second, true));
					assert_equal(s2, *get_icon(f2.second, true));
				}


				test( SettingCaptionIconMakesItAvailableToTheSystem )
				{
					// INIT
					form_and_handle f1(create_form_with_handle()), f2(create_form_with_handle());
					gcontext::surface_type s1(10, 10, 0), s2(16, 16, 0);

					(s1.row_ptr(1) + 2)->components[0] = 0x12;
					(s1.row_ptr(3) + 7)->components[1] = 0x54;
					(s2.row_ptr(10) + 8)->components[3] = 0xF1;

					// ACT
					f1.first->set_caption_icon(s1);
					f2.first->set_caption_icon(s2);

					// ASSERT
					assert_equal(s1, *get_icon(f1.second, false));
					assert_equal(s2, *get_icon(f2.second, false));
				}


				test( CreatedChildFormIsOwnedByTheCreatorForm )
				{
					// INIT
					form_and_handle f1(create_form_with_handle()), f2(create_form_with_handle());
					window_tracker wt(L"#32770");

					// ACT
					shared_ptr<form> cf1 = f1.first->create_child();
					wt.checkpoint();

					// ASSERT
					assert_equal(1u, wt.created.size());
					assert_equal(f1.second, GetWindow(wt.created[0], GW_OWNER));

					// ACT
					shared_ptr<form> cf2 = f2.first->create_child();
					wt.checkpoint();
					shared_ptr<form> cf3 = f1.first->create_child();
					wt.checkpoint();

					// ASSERT
					assert_equal(3u, wt.created.size());
					assert_equal(f2.second, GetWindow(wt.created[1], GW_OWNER));
					assert_equal(f1.second, GetWindow(wt.created[2], GW_OWNER));
				}


				test( SettingLocationMovesWindow )
				{
					// INIT
					form_and_handle f(create_form_with_handle());
					view_location l = { 10, 11, 200, 91 };
					RECT rc;

					// ACT
					f.first->set_location(l);

					// ASSERT
					RECT reference1 = { 10, 11, 10 + 200, 11 + 91 };

					::GetWindowRect(f.second, &rc);
					assert_equal(reference1, rc);

					// ACT
					l.left = 201, l.top = 60, l.width = 400, l.height = 250;
					f.first->set_location(l);

					// ASSERT
					RECT reference2 = { 201, 60, 201 + 400, 60 + 250 };

					::GetWindowRect(f.second, &rc);
					assert_equal(reference2, rc);
				}


				test( CurrentLocationIsReturned )
				{
					// INIT
					form_and_handle f(create_form_with_handle());

					// ACT
					::MoveWindow(f.second, 107, 51, 231, 157, TRUE);

					// ASSERT
					view_location reference1 = { 107, 51, 231, 157 };

					assert_equal(reference1, f.first->get_location());

					// ACT
					::MoveWindow(f.second, 519, 151, 500, 321, TRUE);

					// ASSERT
					view_location reference2 = { 519, 151, 500, 321 };

					assert_equal(reference2, f.first->get_location());
				}

			end_test_suite
		}
	}
}
