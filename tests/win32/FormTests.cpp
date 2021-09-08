#include <wpl/win32/form.h>

#include <wpl/win32/font_manager.h>
#include <wpl/win32/native_view.h>

#include "helpers-win32.h"
#include "MockupsNative.h"

#include <tests/common/helpers-visual.h>
#include <tests/common/mock-control.h>
#include <tests/common/Mockups.h>

#include <tchar.h>
#include <ut/assert.h>
#include <ut/test.h>
#include <windows.h>
#include <wpl/win32/controls.h>

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
				return shared_ptr<void>(::CreateWindowW(L"static", NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hparent,
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
		}

		begin_test_suite( FormTests )
			form_context context;
			window_manager windowManager;
			shared_ptr<win32::font_manager> font_manager;

			form_and_handle create_form_with_handle(HWND howner = 0)
			{
				window_tracker wt(L"#32770");

				shared_ptr<form> f(new win32::form(context, howner));

				wt.checkpoint();

				if (wt.created.size() != 1)
					throw runtime_error("Unexpected amount of windows created!");
				return make_pair(f, wt.created[0]);
			}

			init( Init )
			{
				context.backbuffer.reset(new gcontext::surface_type(1, 1, 16));
				context.renderer.reset(new gcontext::renderer_type(1));
				context.text_engine = create_text_engine();
				windowManager.create_window();
				font_manager = make_shared<win32::font_manager>();
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
				shared_ptr<form> f1(new win32::form(context));

				// ASSERT
				wt.checkpoint();

				assert_equal(1u, wt.created.size());
				assert_is_empty(wt.destroyed);

				// ACT
				shared_ptr<form> f2(new win32::form(context));
				shared_ptr<form> f3(new win32::form(context));

				// ASSERT
				wt.checkpoint();

				assert_equal(3u, wt.created.size());
				assert_is_empty(wt.destroyed);
			}


			test( FormDestructionDestroysItsWindow )
			{
				// INIT
				shared_ptr<form> f1(new win32::form(context));
				shared_ptr<form> f2(new win32::form(context));
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
				f.first->set_caption("Dialog #1...");

				// ASSERT
				assert_equal(L"Dialog #1...", get_window_text(f.second));

				// ACT
				f.first->set_caption("Are you sure?");

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
				const auto ctl = make_shared<mocks::control>();
				form_and_handle f(create_form_with_handle());

				f.first->set_root(ctl);
				ctl->size_log.clear();

				// ACT
				::MoveWindow(f.second, 0, 0, 117, 213, TRUE);

				// ASSERT
				assert_equal(1u, ctl->size_log.size());
				assert_equal(get_client_size(f.second), ctl->size_log.back());

				// ACT
				::MoveWindow(f.second, 27, 190, 531, 97, TRUE);

				// ASSERT
				assert_equal(2u, ctl->size_log.size());
				assert_equal(get_client_size(f.second), ctl->size_log.back());
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
				shared_ptr<form> f(new win32::form(context));
				const auto v = make_shared< mocks::logging_visual<view> >();
				const placed_view pv = {	v, shared_ptr<native_view>(), create_rect(0, 0, 1000, 1000)	};
				const auto ctl = make_shared<mocks::control>();
				rect_i l = { 10, 11, 200, 91 };

				ctl->views.push_back(pv);
				f->set_visible(true);
				wt.checkpoint();
				wt.created.clear();

				// ACT
				auto fc = f->create_child(); 
				fc->set_root(ctl);
				fc->set_location(l);
				fc->set_visible(true);
				wt.checkpoint();
				::RedrawWindow(wt.created[0], NULL, NULL, RDW_UPDATENOW);

				// ASSERT
				assert_equal(1u, v->text_engines_log.size());
				assert_equal(context.text_engine.get(), v->text_engines_log[0]);

				assert_equal(1u, v->surface_size_log.size());
				assert_equal(v->surface_size_log.back().first, static_cast<int>(context.backbuffer->width()));
				assert_equal(v->surface_size_log.back().second, static_cast<int>(context.backbuffer->height()));
			}


			test( SettingLocationMovesWindow )
			{
				// INIT
				form_and_handle f(create_form_with_handle());
				rect_i l = { 10, 11, 200, 91 };
				RECT rc;

				// ACT
				f.first->set_location(l);

				// ASSERT
				RECT reference1 = { 10, 11, 200, 91 };

				::GetWindowRect(f.second, &rc);
				assert_equal(reference1, rc);

				// ACT
				l = create_rect(201, 60, 400, 250);
				f.first->set_location(l);

				// ASSERT
				RECT reference2 = { 201, 60, 400, 250 };

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
				assert_equal(create_rect(107, 51, 338, 208), f.first->get_location());

				// ACT
				::MoveWindow(f.second, 519, 151, 500, 321, TRUE);

				// ASSERT
				assert_equal(create_rect(519, 151, 1019, 472), f.first->get_location());
			}


			test( SetFormStyleAffectsWindowStyleFlags )
			{
				const int mask = WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

				// INIT
				form_and_handle f(create_form_with_handle());

				// ACT
				f.first->set_features(0);

				// ASSERT
				assert_equal(0, mask & ::GetWindowLong(f.second, GWL_STYLE));

				// ACT
				f.first->set_features(form::resizeable);

				// ASSERT
				assert_equal(WS_SIZEBOX, mask & ::GetWindowLong(f.second, GWL_STYLE));

				// ACT
				f.first->set_features(form::resizeable | form::minimizable);

				// ASSERT
				assert_equal(WS_SIZEBOX | WS_MINIMIZEBOX, mask & ::GetWindowLong(f.second, GWL_STYLE));

				// ACT
				f.first->set_features(form::minimizable);

				// ASSERT
				assert_equal(WS_MINIMIZEBOX, mask & ::GetWindowLong(f.second, GWL_STYLE));

				// ACT
				f.first->set_features(form::maximizable);

				// ASSERT
				assert_equal(WS_MAXIMIZEBOX, mask & ::GetWindowLong(f.second, GWL_STYLE));

				// ACT
				f.first->set_features(form::resizeable | form::maximizable | form::minimizable);

				// ASSERT
				assert_equal(WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
					mask & ::GetWindowLong(f.second, GWL_STYLE));
			}


			test( UnderlyingSurfaceIsResizedToClientArea )
			{
				// INIT
				window_tracker wt(L"#32770");
				shared_ptr<form> f(new win32::form(context));
				const auto v = make_shared< mocks::logging_visual<view> >();
				const placed_view pv = {	v, shared_ptr<native_view>(), create_rect(0, 0, 1000, 1000)	};
				const auto ctl = make_shared<mocks::control>();
				rect_i l = { 10, 11, 200, 91 };

				ctl->views.push_back(pv);

				// ACT
				f->set_root(ctl);
				f->set_location(l);
				f->set_visible(true);
				wt.checkpoint();
				::RedrawWindow(wt.created[0], NULL, NULL, RDW_UPDATENOW);

				// ASSERT
				assert_equal(1u, v->text_engines_log.size());
				assert_equal(context.text_engine.get(), v->text_engines_log[0]);

				assert_equal(1u, v->surface_size_log.size());
				assert_equal(v->surface_size_log.back().first, static_cast<int>(context.backbuffer->width()));
				assert_equal(v->surface_size_log.back().second, static_cast<int>(context.backbuffer->height()));

				// INIT
				size_t n_previous = v->surface_size_log.size();
				rect_i l2 = { 10, 11, 230, 97 };

				// ACT
				f->set_location(l2);
				::RedrawWindow(wt.created[0], NULL, NULL, RDW_UPDATENOW);

				// ASSERT
				assert_equal(n_previous + 1u, v->surface_size_log.size());
				assert_equal(v->surface_size_log.back().first, static_cast<int>(context.backbuffer->width()));
				assert_equal(v->surface_size_log.back().second, static_cast<int>(context.backbuffer->height()));
			}


			test( TopLevelGainsFocusEvenWhenWindowedViewsPresent )
			{
				// INIT
				form_and_handle f1(create_form_with_handle());
				form_and_handle f2(create_form_with_handle());

				const placed_view pv[] = {
					{	nullptr, make_shared<mocks::native_view_window>(), create_rect(0, 0, 100, 100)	},
					{	nullptr, make_shared<mocks::native_view_window>(), create_rect(0, 0, 100, 100)	},
				};
				const shared_ptr<mocks::control> ctl[] = {
					make_shared<mocks::control>(), make_shared<mocks::control>(),
				};

				ctl[0]->views.push_back(pv[0]);
				ctl[1]->views.push_back(pv[1]);

				f1.first->set_root(ctl[0]);
				f2.first->set_root(ctl[1]);

				// ACT
				::SetFocus(f1.second);

				// ASSERT
				assert_equal(f1.second, ::GetFocus());

				// ACT
				::SetFocus(f2.second);

				// ASSERT
				assert_equal(f2.second, ::GetFocus());
			}


			test( FormIsCenteredOnRequest )
			{
				// INIT
				form_and_handle parent(create_form_with_handle());
				auto child = parent.first->create_child();

				parent.first->set_location(create_rect(10, 45, 173, 150));
				child->set_location(create_rect(60, 400, 240, 490));

				// ACT
				child->center_parent();

				// ASSERT
				assert_equal(create_rect(1, 52, 181, 142), child->get_location());

				// INIT
				parent.first->set_location(create_rect(210, 345, 453, 700));

				// ACT
				child->center_parent();

				// ASSERT
				assert_equal(create_rect(241, 477, 421, 567), child->get_location());
			}

		end_test_suite
	}
}
