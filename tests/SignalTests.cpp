#include <wpl/signals.h>

#include <string>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			int s1_n;
			int s1x_n;
			int s2_n, s2_arg1;
			int s3_n;
			string s3_arg1;
			int s4_n, s4_arg1, s4_arg2;
			int s5_n, s5_arg1;
			string s5_arg2;
			int s6_n, s6_arg1, s6_arg2, s6_arg3;
			int s7_n, s7_arg1, s7_arg2;
			string s7_arg3;
			int s8_n, s8_arg1, s8_arg2, s8_arg3, s8_arg4;
			int s9_n, s9_arg1, s9_arg2, s9_arg3;
			string s9_arg4;
			int s10_n, s10_arg1, s10_arg2, s10_arg3, s10_arg5;
			const int *s10_arg4;
			int s11_n, s11_arg1, s11_arg2, s11_arg3, s11_arg4;
			const string *s11_arg5;

			void s1_f()
			{	++s1_n;	}

			void s1x_f()
			{	++s1x_n;	}

			void s2_f(int arg1)
			{	++s2_n, s2_arg1 = arg1;	}

			void s3_f(const string &arg1)
			{	++s3_n, s3_arg1 = arg1;	}

			void s4_f(int arg1, int arg2)
			{	++s4_n, s4_arg1 = arg1, s4_arg2 = arg2;	}

			void s5_f(int arg1, const string &arg2)
			{	++s5_n, s5_arg1 = arg1, s5_arg2 = arg2;	}

			void s6_f(int arg1, int arg2, int arg3)
			{	++s6_n, s6_arg1 = arg1, s6_arg2 = arg2, s6_arg3 = arg3;	}

			void s7_f(int arg1, int arg2, const string &arg3)
			{	++s7_n, s7_arg1 = arg1, s7_arg2 = arg2, s7_arg3 = arg3;	}

			void s8_f(int arg1, int arg2, int arg3, int arg4)
			{	++s8_n, s8_arg1 = arg1, s8_arg2 = arg2, s8_arg3 = arg3, s8_arg4 = arg4;	}

			void s9_f(int arg1, int arg2, int arg3, const string &arg4)
			{	++s9_n, s9_arg1 = arg1, s9_arg2 = arg2, s9_arg3 = arg3, s9_arg4 = arg4;	}

			void s10_f(int arg1, int arg2, int arg3, const int &arg4, int arg5)
			{	++s10_n, s10_arg1 = arg1, s10_arg2 = arg2, s10_arg3 = arg3, s10_arg4 = &arg4, s10_arg5 = arg5;	}

			void s11_f(int arg1, int arg2, int arg3, int arg4, const string &arg5)
			{	++s11_n, s11_arg1 = arg1, s11_arg2 = arg2, s11_arg3 = arg3, s11_arg4 = arg4, s11_arg5 = &arg5;	}
		}

		begin_test_suite( SignalTests )
			init( Init )
			{
				s1_n = s1x_n = s2_n = s3_n = s4_n = s5_n = s6_n = s7_n = s8_n = s9_n = s10_n = s11_n = 0;
				s2_arg1 = 0;
				s3_arg1 = string();
				s4_arg1 = s4_arg2 = 0;
				s5_arg1 = 0; s5_arg2 = string();
				s6_arg1 = s6_arg2 = s6_arg3 = 0;
				s7_arg1 = s7_arg2 = 0; s7_arg3 = string();
				s8_arg1 = s8_arg2 = s8_arg3 = s8_arg4 = 0;
				s9_arg1 = s9_arg2 = s9_arg3 = 0; s9_arg4 = string();
				s10_arg1 = s10_arg2 = s10_arg3 = s10_arg5 = 0; s10_arg4 = 0;
				s11_arg1 = s11_arg2 = s11_arg3 = s11_arg4 = 0; s11_arg5 = 0;
			}

			test( InstantiateDifferentSignalTypes )
			{
				// INIT / ACT / ASSERT (compilable)
				signal<void ()> s1;
				signal<void (int)> s2;
				signal<void (string)> s3;
				signal<void (int, int)> s4;
				signal<void (int, string)> s5;
				signal<void (int, int, int)> s6;
				signal<void (int, int, string)> s7;
				signal<void (int, int, int, int)> s8;
				signal<void (int, int, int, string)> s9;
				signal<void (int, int, int, const int &, int)> s10;
				signal<void (int, int, int, int, const string &)> s11;

				// ACT / ASSERT (compilable, nonthrow)
				s1();
				s2(123);
				s3("abc");
				s4(123, 234);
				s5(234, "bcd");
				s6(345, 456, 567);
				s7(456, 567, "cde");
				s8(567, 678, 789, 890);
				s9(678, 789, 890, "def");
				s10(789, 890, 901, 1001, 2001);
				s11(890, 901, 1001, 2001, "efg");
			}


			test( ConnectAndCallSignalReceiver )
			{
				// INIT
				vector<slot_connection> cx;
				signal<void ()> s1;
				signal<void (int)> s2;
				signal<void (string)> s3;
				signal<void (int, int)> s4;
				signal<void (int, string)> s5;
				signal<void (int, int, int)> s6;
				signal<void (int, int, string)> s7;
				signal<void (int, int, int, int)> s8;
				signal<void (int, int, int, string)> s9;
				signal<void (int, int, int, const int &, int)> s10;
				signal<void (int, int, int, int, const string &)> s11;
				int a = 3322;
				string b = "doodle";

				cx.push_back(s1 += &s1_f);
				cx.push_back(s2 += &s2_f);
				cx.push_back(s2 += &s2_f);	// will be called twice
				cx.push_back(s3 += &s3_f);
				cx.push_back(s3 += &s3_f);
				cx.push_back(s3 += &s3_f);	// will be called three times
				cx.push_back(s4 += &s4_f);
				cx.push_back(s5 += &s5_f);
				cx.push_back(s6 += &s6_f);
				cx.push_back(s7 += &s7_f);
				cx.push_back(s8 += &s8_f);
				cx.push_back(s9 += &s9_f);
				cx.push_back(s10 += &s10_f);
				cx.push_back(s11 += &s11_f);

				// ACT
				s1();
				s2(123);
				s3("abc");
				s4(123, 234);
				s5(234, "bcd");
				s6(345, 456, 567);
				s7(456, 567, "cde");
				s8(567, 678, 789, 890);
				s9(678, 789, 890, "def");
				s10(789, 890, 901, a, 1001);
				s11(890, 901, 1001, 2001, b);

				// ASSERT
				assert_equal(1, s1_n);
				assert_equal(2, s2_n);
				assert_equal(123, s2_arg1);
				assert_equal(3, s3_n);
				assert_equal("abc", s3_arg1);
				assert_equal(1, s4_n);
				assert_equal(123, s4_arg1);
				assert_equal(234, s4_arg2);
				assert_equal(1, s5_n);
				assert_equal(234, s5_arg1);
				assert_equal("bcd", s5_arg2);
				assert_equal(1, s6_n);
				assert_equal(345, s6_arg1);
				assert_equal(456, s6_arg2);
				assert_equal(567, s6_arg3);
				assert_equal(1, s7_n);
				assert_equal(456, s7_arg1);
				assert_equal(567, s7_arg2);
				assert_equal("cde", s7_arg3);
				assert_equal(1, s8_n);
				assert_equal(567, s8_arg1);
				assert_equal(678, s8_arg2);
				assert_equal(789, s8_arg3);
				assert_equal(890, s8_arg4);
				assert_equal(1, s9_n);
				assert_equal(678, s9_arg1);
				assert_equal(789, s9_arg2);
				assert_equal(890, s9_arg3);
				assert_equal("def", s9_arg4);
				assert_equal(1, s10_n);
				assert_equal(789, s10_arg1);
				assert_equal(890, s10_arg2);
				assert_equal(901, s10_arg3);
				assert_equal(&a, s10_arg4);
				assert_equal(1001, s10_arg5);
				assert_equal(1, s11_n);
				assert_equal(890, s11_arg1);
				assert_equal(901, s11_arg2);
				assert_equal(1001, s11_arg3);
				assert_equal(2001, s11_arg4);
				assert_equal(&b, s11_arg5);
			}


			test( DisconnectedSlotIsNotCalled )
			{
				// INIT
				signal<void ()> s;
				slot_connection c2;

				// ACT
				{
					slot_connection c(s += &s1_f);
					c2 = s += s1x_f;
				}
				s();

				// ASSERT
				assert_equal(0, s1_n);
				assert_equal(1, s1x_n);
			}


			test( DisconnectedSlotIsNotCalledForAllSignatures )
			{
				// INIT
				signal<void ()> s1;
				signal<void (int)> s2;
				signal<void (int, int)> s4;
				signal<void (int, int, int)> s6;
				signal<void (int, int, int, int)> s8;
				signal<void (int, int, int, const int &, int)> s10;

				// ACT (connect and disconnect at statement completion)
				s1 += &s1_f;
				s2 += &s2_f;
				s4 += &s4_f;
				s6 += &s6_f;
				s8 += &s8_f;
				s10 += &s10_f;

				// ACT / ASSERT (must not throw)
				s1();
				s2(123);
				s4(123, 234);
				s6(345, 456, 567);
				s8(567, 678, 789, 890);
				s10(789, 890, 901, 2002, 1001);

				// ASSERT
				assert_equal(0, s1_n);
				assert_equal(0, s2_n);
				assert_equal(0, s4_n);
				assert_equal(0, s6_n);
				assert_equal(0, s8_n);
				assert_equal(0, s10_n);
			}


			test( SlotConnectionIsAssignable )
			{
				// INIT
				signal<void ()> s;
				slot_connection c2;

				// ACT
				{
					slot_connection c(s += &s1_f);

					c2 = c;
				}
				s();

				// ASSERT
				assert_equal(1, s1_n);
			}


			test( CopyingOfSignalDoesNotCopyConnectedSlots )
			{
				// INIT
				signal<void ()> s_src;
				slot_connection c(s_src += &s1_f);

				// ACT
				signal<void ()> s_clone(s_src);
				
				s_clone();

				// ASSERT
				assert_equal(0, s1_n);
			}


			test( CopiedSignalIsWorkable )
			{
				// INIT
				signal<void ()> s_src;

				// ACT
				signal<void ()> s_clone(s_src);
				slot_connection c(s_clone += &s1_f);	// must not throw
				
				s_clone();	// must not throw

				// ASSERT
				assert_equal(1, s1_n);
			}


			test( AssigningOfSignalDoesNotAssignConnectedSlotsAndContinueToCallOldSlots )
			{
				// INIT
				signal<void ()> s_src;
				signal<void ()> s_clone;
				slot_connection c1(s_src += &s1_f);
				slot_connection c2(s_clone += &s1x_f);

				// ACT
				s_clone = s_src;
				s_clone();

				// ASSERT
				assert_equal(0, s1_n);
				assert_equal(1, s1x_n);
			}


			test( AssigningReturnsConstThis )
			{
				// INIT
				signal<void ()> s_src;
				signal<void ()> s_clone;

				// ACT
				signal<void ()> *ref = &(s_clone = s_src);

				// ASSERT
				assert_equal(ref, &s_clone);
			}


			test( SelfDestructionIsSupportedFromSignal0 )
			{
				// INIT
				typedef signal<void ()> signal_type;

				unique_ptr<signal_type> ps(new signal_type());
				slot_connection c1(*ps += bind(&unique_ptr<signal_type>::reset, &ps, static_cast<signal_type *>(0)));
				slot_connection c2(*ps += s1_f);
				
				// ACT / ASSERT (must not throw)
				(*ps)();

				// ASSERT
				assert_equal(1, s1_n);
				assert_null(ps.get());
			}


			test( SelfDestructionIsSupportedFromSignal1 )
			{
				// INIT
				typedef signal<void (int)> signal_type;

				unique_ptr<signal_type> ps(new signal_type());
				slot_connection c1(*ps += bind(&unique_ptr<signal_type>::reset, &ps, static_cast<signal_type *>(0)));
				slot_connection c2(*ps += s2_f);
				
				// ACT / ASSERT (must not throw)
				(*ps)(0);

				// ASSERT
				assert_equal(1, s2_n);
				assert_null(ps.get());
			}


			test( SelfDestructionIsSupportedFromSignal2 )
			{
				// INIT
				typedef signal<void (int, int)> signal_type;

				unique_ptr<signal_type> ps(new signal_type());
				slot_connection c1(*ps += bind(&unique_ptr<signal_type>::reset, &ps, static_cast<signal_type *>(0)));
				slot_connection c2(*ps += s4_f);
				
				// ACT / ASSERT (must not throw)
				(*ps)(0, 0);

				// ASSERT
				assert_equal(1, s4_n);
				assert_null(ps.get());
			}


			test( SelfDestructionIsSupportedFromSignal3 )
			{
				// INIT
				typedef signal<void (int, int, int)> signal_type;

				unique_ptr<signal_type> ps(new signal_type());
				slot_connection c1(*ps += bind(&unique_ptr<signal_type>::reset, &ps, static_cast<signal_type *>(0)));
				slot_connection c2(*ps += s6_f);
				
				// ACT / ASSERT (must not throw)
				(*ps)(0, 0, 0);

				// ASSERT
				assert_equal(1, s6_n);
				assert_null(ps.get());
			}


			test( SelfDestructionIsSupportedFromSignal4 )
			{
				// INIT
				typedef signal<void (int, int, int, int)> signal_type;

				unique_ptr<signal_type> ps(new signal_type());
				slot_connection c1(*ps += bind(&unique_ptr<signal_type>::reset, &ps, static_cast<signal_type *>(0)));
				slot_connection c2(*ps += s8_f);
				
				// ACT / ASSERT (must not throw)
				(*ps)(0, 0, 0, 0);

				// ASSERT
				assert_equal(1, s8_n);
				assert_null(ps.get());
			}


			test( SelfDestructionIsSupportedFromSignal5 )
			{
				// INIT
				typedef signal<void (int, int, int, int, int)> signal_type;

				unique_ptr<signal_type> ps(new signal_type());
				slot_connection c1(*ps += bind(&unique_ptr<signal_type>::reset, &ps, static_cast<signal_type *>(0)));
				slot_connection c2(*ps += s10_f);
				
				// ACT / ASSERT (must not throw)
				(*ps)(0, 0, 0, 0, 0);

				// ASSERT
				assert_equal(1, s10_n);
				assert_null(ps.get());
			}
		end_test_suite
	}
}
