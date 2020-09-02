#include <wpl/form.h>
#include <wpl/win32/native_view.h>
#include <wpl/win32/form.h>

#include "helpers-visual.h"
#include "helpers-win32.h"
#include "Mockups.h"
#include "MockupsNative.h"

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
	namespace tests
	{
		namespace
		{
			typedef pair<shared_ptr<form>, HWND> form_and_handle;

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
				union
				{
					BITMAPINFO bi;
					char space[sizeof(BITMAPINFO) + 3 * sizeof(DWORD)];
				} bi = {};

				bi.bi.bmiHeader.biSize = sizeof(bi.bi.bmiHeader);
				::GetDIBits(static_cast<HDC>(hdc.get()), ii.hbmColor, 0, b.bmHeight, NULL, &bi.bi, DIB_RGB_COLORS);
				bi.bi.bmiHeader.biHeight = -bi.bi.bmiHeader.biHeight;
				::GetDIBits(static_cast<HDC>(hdc.get()), ii.hbmColor, 0, b.bmHeight, buffer.get(), &bi.bi, DIB_RGB_COLORS);

				shared_ptr<gcontext::surface_type> s(new gcontext::surface_type(b.bmWidth, b.bmHeight, 0));
				memcpy(s->row_ptr(0), buffer.get(), b.bmHeight * b.bmWidthBytes);
				return s;
			}

			font create_font(const wstring &typeface, int height)
			{
				font f = { typeface, height };
				return f;
			}

			int convert_font_height(int height)
			{
				const shared_ptr<void> hdc(::CreateCompatibleDC(NULL), &::DeleteDC);
				return -::MulDiv(height, ::GetDeviceCaps(static_cast<HDC>(hdc.get()), LOGPIXELSY), 72);
			}
		}

		begin_test_suite( FormTests )
			mocks::font_loader fake_loader;
			shared_ptr<gcontext::surface_type> surface;
			shared_ptr<gcontext::renderer_type> renderer;
			shared_ptr<gcontext::text_engine_type> text_engine;
			WindowManager windowManager;

			form_and_handle create_form_with_handle(HWND howner = 0)
			{
				window_tracker wt(L"#32770");

				shared_ptr<form> f(new win32::form(surface, renderer, text_engine, howner));

				wt.checkpoint();

				if (wt.created.size() != 1)
					throw runtime_error("Unexpected amount of windows created!");
				return make_pair(f, wt.created[0]);
			}

			init( Init )
			{
				surface.reset(new gcontext::surface_type(1, 1, 16));
				renderer.reset(new gcontext::renderer_type(1));
				text_engine.reset(new gcontext::text_engine_type(fake_loader, 0));
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
				shared_ptr<form> f1(new win32::form(surface, renderer, shared_ptr<gcontext::text_engine_type>()));

				// ASSERT
				wt.checkpoint();

				assert_equal(1u, wt.created.size());
				assert_is_empty(wt.destroyed);

				// ACT
				shared_ptr<form> f2(new win32::form(surface, renderer, shared_ptr<gcontext::text_engine_type>()));
				shared_ptr<form> f3(new win32::form(surface, renderer, shared_ptr<gcontext::text_engine_type>()));

				// ASSERT
				wt.checkpoint();

				assert_equal(3u, wt.created.size());
				assert_is_empty(wt.destroyed);
			}


			test( FormDestructionDestroysItsWindow )
			{
				// INIT
				shared_ptr<form> f1(new win32::form(surface, renderer, shared_ptr<gcontext::text_engine_type>()));
				shared_ptr<form> f2(new win32::form(surface, renderer, shared_ptr<gcontext::text_engine_type>()));
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


			test( ChildFormInheritsParentSurface )
			{
				// INIT
				window_tracker wt(L"#32770");
				shared_ptr<form> f(new win32::form(surface, renderer, text_engine));
				shared_ptr< mocks::logging_visual<view> > v(new mocks::logging_visual<view>);
				view_location l = { 10, 11, 200, 91 };

				f->set_visible(true);
				wt.checkpoint();
				wt.created.clear();

				// ACT
				auto fc = f->create_child(); 
				fc->set_view(v);
				fc->set_location(l);
				fc->set_visible(true);
				wt.checkpoint();
				::RedrawWindow(wt.created[0], NULL, NULL, RDW_UPDATENOW);

				// ASSERT
				assert_equal(1u, v->text_engines_log.size());
				assert_equal(text_engine.get(), v->text_engines_log[0]);

				assert_equal(1u, v->surface_size_log.size());
				assert_equal(v->surface_size_log.back().first, static_cast<int>(surface->width()));
				assert_equal(v->surface_size_log.back().second, static_cast<int>(surface->height()));
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


			test( SetFormStyleAffectsWindowStyleFlags )
			{
				const int mask = WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

				// INIT
				form_and_handle f(create_form_with_handle());

				// ACT
				f.first->set_style(0);

				// ASSERT
				assert_equal(0, mask & ::GetWindowLong(f.second, GWL_STYLE));

				// ACT
				f.first->set_style(form::resizeable);

				// ASSERT
				assert_equal(WS_SIZEBOX, mask & ::GetWindowLong(f.second, GWL_STYLE));

				// ACT
				f.first->set_style(form::resizeable | form::has_minimize);

				// ASSERT
				assert_equal(WS_SIZEBOX | WS_MINIMIZEBOX, mask & ::GetWindowLong(f.second, GWL_STYLE));

				// ACT
				f.first->set_style(form::has_minimize);

				// ASSERT
				assert_equal(WS_MINIMIZEBOX, mask & ::GetWindowLong(f.second, GWL_STYLE));

				// ACT
				f.first->set_style(form::has_maximize);

				// ASSERT
				assert_equal(WS_MAXIMIZEBOX, mask & ::GetWindowLong(f.second, GWL_STYLE));

				// ACT
				f.first->set_style(form::resizeable | form::has_maximize | form::has_minimize);

				// ASSERT
				assert_equal(WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
					mask & ::GetWindowLong(f.second, GWL_STYLE));
			}


			test( RequestedFontIsUnderlyingWindow )
			{
				// INIT
				form_and_handle f(create_form_with_handle());

				// ACT
				f.first->set_font(create_font(L"Arial", 10));

				// ASSERT
				HFONT hfont = reinterpret_cast<HFONT>(::SendMessage(f.second, WM_GETFONT, 0, 0));
				LOGFONT lf = {};

				::GetObject(hfont, sizeof(lf), &lf);
				assert_equal(_T("Arial"), tstring(lf.lfFaceName));
				assert_equal(convert_font_height(10), lf.lfHeight);

				// ACT
				f.first->set_font(create_font(L"Times New Roman", 20));

				// ASSERT
				hfont = reinterpret_cast<HFONT>(::SendMessage(f.second, WM_GETFONT, 0, 0));

				::GetObject(hfont, sizeof(lf), &lf);
				assert_equal(_T("Times New Roman"), tstring(lf.lfFaceName));
				assert_equal(convert_font_height(20), lf.lfHeight);
			}


			test( UnderlyingSurfaceIsResizedToClientArea )
			{
				// INIT
				window_tracker wt(L"#32770");
				shared_ptr<form> f(new win32::form(surface, renderer, text_engine));
				shared_ptr< mocks::logging_visual<view> > v(new mocks::logging_visual<view>);
				view_location l = { 10, 11, 200, 91 };

				// ACT
				f->set_view(v);
				f->set_location(l);
				f->set_visible(true);
				wt.checkpoint();
				::RedrawWindow(wt.created[0], NULL, NULL, RDW_UPDATENOW);

				// ASSERT
				assert_equal(1u, v->text_engines_log.size());
				assert_equal(text_engine.get(), v->text_engines_log[0]);

				assert_equal(1u, v->surface_size_log.size());
				assert_equal(v->surface_size_log.back().first, static_cast<int>(surface->width()));
				assert_equal(v->surface_size_log.back().second, static_cast<int>(surface->height()));

				// INIT
				size_t n_previous = v->surface_size_log.size();
				view_location l2 = { 10, 11, 230, 97 };

				// ACT
				f->set_location(l2);
				::RedrawWindow(wt.created[0], NULL, NULL, RDW_UPDATENOW);

				// ASSERT
				assert_equal(n_previous + 1u, v->surface_size_log.size());
				assert_equal(v->surface_size_log.back().first, static_cast<int>(surface->width()));
				assert_equal(v->surface_size_log.back().second, static_cast<int>(surface->height()));
			}


			test( TheWholeWindowIsInvalidatedOnSettingBackgroundColor )
			{
				// INIT
				form_and_handle f(create_form_with_handle());
				view_location l = { 10, 11, 200, 91 };

				f.first->set_location(l);
				f.first->set_visible(true);
				::ValidateRect(f.second, NULL);

				// ACT
				f.first->set_background_color(agge::color::make(200, 150, 100));

				// ASSERT
				RECT reference, invalid;

				::GetClientRect(f.second, &reference);
				assert_is_true(!!::GetUpdateRect(f.second, &invalid, FALSE));
				assert_equal(reference, invalid);

				// INIT
				l.width++, l.height = 250;
				f.first->set_location(l);
				::ValidateRect(f.second, NULL);

				// ACT
				f.first->set_background_color(agge::color::make(200, 150, 100));

				// ASSERT
				::GetClientRect(f.second, &reference);
				assert_is_true(!!::GetUpdateRect(f.second, &invalid, FALSE));
				assert_equal(reference, invalid);
			}


			test( BackgroundIsFilledWithPresetColor )
			{
				// INIT
				form_and_handle f(create_form_with_handle());
				shared_ptr< mocks::logging_visual<view> > v(new mocks::logging_visual<view>);
				view_location l = { 100, 100, 200, 150 };

				f.first->set_location(l);
				f.first->set_view(v);
				f.first->set_visible(true);
				::ValidateRect(f.second, NULL);

				// ACT
				f.first->set_background_color(agge::color::make(200, 150, 100));
				::RedrawWindow(f.second, NULL, NULL, RDW_UPDATENOW);

				// ASSERT
				pair<gcontext::pixel_type, bool> reference1[] = {
					make_pair(make_pixel(agge::color::make(200, 150, 100, 255)), true),
				};

				assert_equal(reference1, v->background_color);

				// ACT
				f.first->set_background_color(agge::color::make(100, 0, 200));
				::RedrawWindow(f.second, NULL, NULL, RDW_UPDATENOW);

				// ASSERT
				pair<gcontext::pixel_type, bool> reference2[] = {
					make_pair(make_pixel(agge::color::make(200, 150, 100, 255)), true),
					make_pair(make_pixel(agge::color::make(100, 0, 200, 255)), true),
				};

				assert_equal(reference2, v->background_color);
			}

		end_test_suite
	}
}
