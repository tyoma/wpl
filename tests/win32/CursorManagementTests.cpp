#include <wpl/win32/cursor_manager.h>

#include "helpers-win32.h"

#include <tests/common/helpers.h>
#include <tests/common/Mockups.h>

#include <ut/assert.h>
#include <ut/test.h>
#include <windows.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			shared_ptr<void> get_cursor(LPCTSTR id)
			{
				return shared_ptr<void>(static_cast<HCURSOR>(::LoadImage(0, id, IMAGE_CURSOR, 0, 0, LR_SHARED)),
					&::DestroyCursor);
			}
		}

		begin_test_suite( CursorManagementTests )

			test( CursorManagerReturnsDistinctPredefinedCursors )
			{
				// INIT / ACT
				win32::cursor_manager cm;
				shared_ptr<const cursor> cursors[] = {
					cm.get(cursor_manager::arrow),
					cm.get(cursor_manager::i_beam),
					cm.get(cursor_manager::crosshair),
					cm.get(cursor_manager::hand),
					cm.get(cursor_manager::h_resize),
					cm.get(cursor_manager::v_resize),
				};

				// ASSERT
				sort(begin(cursors), end(cursors));

				assert_equal(end(cursors), unique(begin(cursors), end(cursors)));
			}


			test( SameCursorObjectsAreReturnedForTheSamePredefinedIDs )
			{
				// INIT
				win32::cursor_manager cm;

				// ACT
				shared_ptr<const cursor> cursors1[] = {
					cm.get(cursor_manager::arrow),
					cm.get(cursor_manager::arrow),
				};
				shared_ptr<const cursor> cursors2[] = {
					cm.get(cursor_manager::i_beam),
					cm.get(cursor_manager::i_beam),
				};
				shared_ptr<const cursor> cursors3[] = {
					cm.get(cursor_manager::crosshair),
					cm.get(cursor_manager::crosshair),
				};
				shared_ptr<const cursor> cursors4[] = {
					cm.get(cursor_manager::hand),
					cm.get(cursor_manager::hand),
				};
				shared_ptr<const cursor> cursors5[] = {
					cm.get(cursor_manager::h_resize),
					cm.get(cursor_manager::h_resize),
				};
				shared_ptr<const cursor> cursors6[] = {
					cm.get(cursor_manager::v_resize),
					cm.get(cursor_manager::v_resize),
				};

				// ASSERT
				assert_equal(cursors1[0], cursors1[1]);
				assert_equal(cursors2[0], cursors2[1]);
				assert_equal(cursors3[0], cursors3[1]);
				assert_equal(cursors4[0], cursors4[1]);
				assert_equal(cursors5[0], cursors5[1]);
				assert_equal(cursors6[0], cursors6[1]);
			}


			test( SettingACursorUpdatesSystemCursor )
			{
				// INIT
				win32::cursor_manager cm;

				// ACT / ASSERT
				cm.set(cm.get(cursor_manager::arrow));
				assert_equal(static_cast<HCURSOR>(get_cursor(IDC_ARROW).get()), ::GetCursor());

				cm.set(cm.get(cursor_manager::i_beam));
				assert_equal(static_cast<HCURSOR>(get_cursor(IDC_IBEAM).get()), ::GetCursor());

				cm.set(cm.get(cursor_manager::hand));
				assert_equal(static_cast<HCURSOR>(get_cursor(IDC_HAND).get()), ::GetCursor());

				cm.set(cm.get(cursor_manager::crosshair));
				assert_equal(static_cast<HCURSOR>(get_cursor(IDC_CROSS).get()), ::GetCursor());

				cm.set(cm.get(cursor_manager::h_resize));
				assert_equal(static_cast<HCURSOR>(get_cursor(IDC_SIZEWE).get()), ::GetCursor());

				cm.set(cm.get(cursor_manager::v_resize));
				assert_equal(static_cast<HCURSOR>(get_cursor(IDC_SIZENS).get()), ::GetCursor());
			}


			test( PushingACursorSetsUpTheNewCursor )
			{
				// INIT
				win32::cursor_manager cm;

				// ACT / ASSERT
				cm.push(cm.get(cursor_manager::arrow));
				assert_equal(static_cast<HCURSOR>(get_cursor(IDC_ARROW).get()), ::GetCursor());

				cm.push(cm.get(cursor_manager::i_beam));
				assert_equal(static_cast<HCURSOR>(get_cursor(IDC_IBEAM).get()), ::GetCursor());

				cm.push(cm.get(cursor_manager::hand));
				assert_equal(static_cast<HCURSOR>(get_cursor(IDC_HAND).get()), ::GetCursor());

				cm.push(cm.get(cursor_manager::crosshair));
				assert_equal(static_cast<HCURSOR>(get_cursor(IDC_CROSS).get()), ::GetCursor());

				cm.push(cm.get(cursor_manager::h_resize));
				assert_equal(static_cast<HCURSOR>(get_cursor(IDC_SIZEWE).get()), ::GetCursor());

				cm.push(cm.get(cursor_manager::v_resize));
				assert_equal(static_cast<HCURSOR>(get_cursor(IDC_SIZENS).get()), ::GetCursor());
			}


			test( SinglePoppingACursorSetsItToAPrevious )
			{
				// INIT
				win32::cursor_manager cm;

				::SetCursor(static_cast<HCURSOR>(get_cursor(IDC_IBEAM).get()));

				cm.push(cm.get(cursor_manager::arrow));

				// ACT / ASSERT
				cm.pop();
				assert_equal(static_cast<HCURSOR>(get_cursor(IDC_IBEAM).get()), ::GetCursor());

				// INIT
				::SetCursor(static_cast<HCURSOR>(get_cursor(IDC_CROSS).get()));

				cm.push(cm.get(cursor_manager::arrow));

				// ACT / ASSERT
				cm.pop();
				assert_equal(static_cast<HCURSOR>(get_cursor(IDC_CROSS).get()), ::GetCursor());
			}


			test( MultiplePoppingACursorSetsItToAPrevious )
			{
				// INIT
				win32::cursor_manager cm;

				::SetCursor(static_cast<HCURSOR>(get_cursor(IDC_IBEAM).get()));

				cm.push(cm.get(cursor_manager::arrow));
				cm.push(cm.get(cursor_manager::crosshair));
				cm.push(cm.get(cursor_manager::v_resize));

				// ACT / ASSERT
				assert_equal(static_cast<HCURSOR>(get_cursor(IDC_SIZENS).get()), ::GetCursor());
				cm.pop();
				assert_equal(static_cast<HCURSOR>(get_cursor(IDC_CROSS).get()), ::GetCursor());
				cm.pop();
				assert_equal(static_cast<HCURSOR>(get_cursor(IDC_ARROW).get()), ::GetCursor());
				cm.pop();
				assert_equal(static_cast<HCURSOR>(get_cursor(IDC_IBEAM).get()), ::GetCursor());
			}


			test( DestructionOfCursorManagerRestoresFirstPushedCursor )
			{
				// INIT
				unique_ptr<win32::cursor_manager> cm(new win32::cursor_manager);
				const auto initial = get_cursor(IDC_IBEAM);

				::SetCursor(static_cast<HCURSOR>(initial.get()));
				cm->push(cm->get(cursor_manager::arrow));
				cm->push(cm->get(cursor_manager::crosshair));
				cm->push(cm->get(cursor_manager::v_resize));

				// ACT
				cm.reset();

				// ASSERT
				assert_equal(static_cast<HCURSOR>(initial.get()), ::GetCursor());
			}


			test( PoppingEmptyStackDoesNothing )
			{
				// INIT
				win32::cursor_manager cm;
				const auto c1 = get_cursor(IDC_IBEAM);
				const auto c2 = get_cursor(IDC_IBEAM);

				::SetCursor(static_cast<HCURSOR>(c2.get()));

				// ACT
				cm.pop();

				// ASSERT
				assert_equal(static_cast<HCURSOR>(c2.get()), ::GetCursor());

				// INIT
				::SetCursor(static_cast<HCURSOR>(c1.get()));

				// ACT
				cm.pop();

				// ASSERT
				assert_equal(static_cast<HCURSOR>(c1.get()), ::GetCursor());
			}
		end_test_suite
	}
}
