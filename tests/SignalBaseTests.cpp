#include <wpl/signal.h>

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			template <typename T>
			struct open_signal : signal<T>
			{
				using signal<T>::for_each_invoke;
			};

			struct slot_state : noncopyable
			{
				slot_state()
					: handles(0), destroyed(false)
				{	}

				int handles;
				bool destroyed;
			};

			bool operator ==(pair<int, bool> lhs, const slot_state &rhs)
			{	return lhs == make_pair(rhs.handles, rhs.destroyed);	}

			struct slot_handle
			{
				slot_handle(const slot_handle &other)
					: state(other.state)
				{
					if (state)
						state->handles++;
				}

				explicit slot_handle(slot_state *state_ = nullptr)
					: state(state_)
				{
					if (state)
						state->handles++;
				}

				~slot_handle()
				{
					if (state && !--(state->handles))
						state->destroyed = true;
				}

				const slot_handle &operator =(const slot_handle &rhs)
				{
					assert_not_null(state);
					assert_null(rhs.state);

					state->destroyed = true;
					return *this;
				}

				operator bool() const
				{
					assert_not_null(state);
					return !state->destroyed;
				}

				slot_state * const state;
			};
		}


		begin_test_suite( SignalBaseTests )
			test( SlotIsHeldByConnection )
			{
				// INIT
				slot_state states[3];
				open_signal<slot_handle> s;

				// INIT / ACT
				slot_connection c1[] = {
					s += slot_handle(&states[0]),
					s += slot_handle(&states[1]),
				};

				// ASSERT
				pair<int, bool> reference1[] = {	make_pair(1, false), make_pair(1, false), make_pair(0, false),	};

				assert_equal(reference1, states);

				// INIT / ACT
				auto c2 = s += slot_handle(&states[2]);

				// ASSERT
				pair<int, bool> reference2[] = {	make_pair(1, false), make_pair(1, false), make_pair(1, false),	};

				assert_equal(reference2, states);

				// ACT
				c1[1] = slot_connection();

				// ASSERT
				pair<int, bool> reference3[] = {	make_pair(1, false), make_pair(0, true), make_pair(1, false),	};

				assert_equal(reference3, states);
			}


			test( StateIsDestroyedButHandleIsAliveDuringInvocation )
			{
				// INIT
				slot_state states[4];
				open_signal<slot_handle> s;
				slot_connection c[] = {
					s += slot_handle(&states[0]),
					s += slot_handle(&states[1]),
					s += slot_handle(&states[2]),
					s += slot_handle(&states[3]),
				};
				auto n = 0;

				// ACT
				s.for_each_invoke([&] (slot_handle &/*h*/) {
					switch (n++)
					{
					case 1:
						c[0] = slot_connection();
						c[2] = slot_connection();

				// ASSERT
						pair<int, bool> reference[] = {
							make_pair(1, true), make_pair(1, false), make_pair(1, true),	make_pair(1, false),
						};

						assert_equal(reference, states);
					}
				});

				// ASSERT
				pair<int, bool> reference1[] = {
					make_pair(0, true), make_pair(1, false), make_pair(0, true),	make_pair(1, false),
				};

				assert_equal(3, n);
				assert_equal(reference1, states);
			}


			test( OnlyAliveStatesAreCalledInANestedInvocation )
			{
				// INIT
				slot_state states[4];
				open_signal<slot_handle> s;
				slot_connection c[] = {
					s += slot_handle(&states[0]),
					s += slot_handle(&states[1]),
					s += slot_handle(&states[2]),
					s += slot_handle(&states[3]),
				};
				auto n = 0;

				// ACT
				s.for_each_invoke([&] (slot_handle &/*h*/) {
					switch (n++)
					{
					case 1:
						c[0] = slot_connection();
						c[3] = slot_connection();
						break;

					case 2:
						s.for_each_invoke([&] (slot_handle &h) {

				// ASSERT
							assert_is_true(&states[1] == h.state || &states[2] == h.state);
						});
						break;
					}
				});
			}


			test( StatesAreDestroyedButHandlesAreKeptAfterANestedInvocation )
			{
				// INIT
				slot_state states[4];
				open_signal<slot_handle> s;
				slot_connection c[] = {
					s += slot_handle(&states[0]),
					s += slot_handle(&states[1]),
					s += slot_handle(&states[2]),
					s += slot_handle(&states[3]),
				};
				auto n = 0;

				// ACT
				s.for_each_invoke([&] (slot_handle &/*h*/) {
					switch (n++)
					{
					case 1:
						s.for_each_invoke([&] (slot_handle &) {
							c[0] = slot_connection();
							c[2] = slot_connection();
						});
						break;

					case 3:
						assert_is_true(false);
						break;

					case 2:

				// ASSERT
						pair<int, bool> reference[] = {
							make_pair(1, true), make_pair(1, false), make_pair(1, true), make_pair(1, false),
						};

						assert_equal(reference, states);
					}
				});
			}
		end_test_suite
	}
}
