#include <wpl/iterator.h>
#include <wpl/view_helpers.h>
#include <wpl/view.h>

#include "common/helpers.h"

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		begin_test_suite( MiscTests )
			test( HasNoFocusOnCreation )
			{
				// INIT / ACT
				on_focus_invalidate<view> v;

				// ASSERT
				assert_is_false(v.has_focus);
			}


			test( FocusInOutSetsFlag )
			{
				// INIT
				on_focus_invalidate<view> v;

				// ACT
				static_cast<keyboard_input &>(v).got_focus();

				// ASSERT
				assert_is_true(v.has_focus);

				// ACT
				static_cast<keyboard_input &>(v).lost_focus();

				// ASSERT
				assert_is_false(v.has_focus);
			}


			test( FocusInOutInvalidatesView )
			{
				// INIT
				auto invalidates = 0;
				on_focus_invalidate<view> v;
				auto c = v.invalidate += [&] (const void *r) {
					assert_null(r);
					invalidates++;
				};

				// ACT
				static_cast<keyboard_input &>(v).got_focus();

				// ASSERT
				assert_equal(1, invalidates);

				// ACT
				static_cast<keyboard_input &>(v).lost_focus();
				static_cast<keyboard_input &>(v).got_focus();

				// ASSERT
				assert_equal(3, invalidates);
			}
		end_test_suite


		begin_test_suite( IteratorUtilityTests )

			template <typename C, typename FromC>
			static C make(const FromC &from)
			{	return C(begin(from), end(from));	}

			template <typename C>
			typename C::const_iterator last(C &container)
			{
				auto i = end(container);
				return --i;
			}


			test( CyclingEmptyContainerFromEndReturnsEnd )
			{
				// INIT
				auto c1 = vector<int>();
				auto c2 = list<string>();

				// ACT / ASSERT
				assert_equal(end(c1), cycle_next(c1, end(c1)));
				assert_equal(end(c2), cycle_next(c2, end(c2)));
				assert_equal(end(c1), cycle_previous(c1, end(c1)));
				assert_equal(end(c2), cycle_previous(c2, end(c2)));
			}


			test( CyclingForwardFromEndSetsIteratorToBegin )
			{
				// INIT
				auto c1 = make< const vector<int> >(plural + 13 + 17 + 191 + 1);
				auto c2 = make< list<string> >(plural
					+ string("lorem") + string("ipsum") + string("amet") + string("dolor") + string("ipsum"));

				// ACT / ASSERT
				assert_equal(begin(c1), cycle_next(c1, end(c1)));
				assert_equal(begin(c2), cycle_next(c2, end(c2)));
			}


			test( CyclingBackwardFromEndSetsIteratorToBegin )
			{
				// INIT
				auto c1 = make< const vector<int> >(plural + 13 + 17 + 191 + 1);
				auto c2 = make< list<string> >(plural
					+ string("lorem") + string("ipsum") + string("amet") + string("dolor") + string("ipsum"));

				// ACT / ASSERT
				assert_equal(last(c1), cycle_previous(c1, end(c1)));
				assert_equal(last(c2), cycle_previous(c2, end(c2)));
			}


			test( CyclingForwardIteratesAndWrapsAround )
			{
				// INIT
				auto c1 = make< const vector<int> >(plural + 13 + 17 + 191 + 1);
				auto c2 = make< list<string> >(plural
					+ string("lorem") + string("ipsum") + string("amet") + string("dolor") + string("ipsum"));
				auto i1 = begin(c1);
				auto i2 = begin(c2);
				auto ii1 = i1++;
				auto ii2 = i2++;

				// ACT / ASSERT
				assert_equal(i1, cycle_next(c1, ii1));
				assert_equal(i2, cycle_next(c2, ii2));

				// INIT
				ii1 = i1++;
				ii2 = i2++;

				// ACT / ASSERT
				assert_equal(i1, cycle_next(c1, ii1));
				assert_equal(i2, cycle_next(c2, ii2));

				// INIT
				ii1 = end(c1), --ii1;
				ii2 = end(c2), --ii2;

				// ACT / ASSERT
				assert_equal(begin(c1), cycle_next(c1, ii1));
				assert_equal(begin(c2), cycle_next(c2, ii2));
			}


			test( CyclingBackwardIteratesAndWrapsAround )
			{
				// INIT
				auto c1 = make< const vector<int> >(plural + 13 + 17 + 191 + 1);
				auto c2 = make< list<string> >(plural
					+ string("lorem") + string("ipsum") + string("amet") + string("dolor") + string("ipsum"));
				auto last1 = end(c1);
				auto last2 = end(c2);

				--last1, --last2;

				auto i1 = last1;
				auto i2 = last2;

				----i1, --i2;

				auto ii1 = i1--;
				auto ii2 = i2--;

				// ACT / ASSERT
				assert_equal(i1, cycle_previous(c1, ii1));
				assert_equal(i2, cycle_previous(c2, ii2));

				// INIT
				ii1 = i1++;
				ii2 = i2++;

				// ACT / ASSERT
				assert_equal(last1, cycle_previous(c1, begin(c1)));
				assert_equal(last2, cycle_previous(c2, begin(c2)));
			}

		end_test_suite
	}
}
