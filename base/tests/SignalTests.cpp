#include <signals.h>

#include <string>

using namespace std;
using namespace System;
using namespace	Microsoft::VisualStudio::TestTools::UnitTesting;

namespace wpl
{
	namespace tests
	{
		[TestClass]
		public ref class SignalTests
		{
		public: 
			[TestMethod]
			void InstantiateDifferentSignalTypes()
			{
				// INIT / ACT / ASSERT (compilable)
				signal<void()> s1;
				signal<void(int)> s2;
				signal<void(string)> s3;
				signal<void(int, int)> s4;
				signal<void(int, string)> s5;
				signal<void(int, int, int)> s6;
				signal<void(int, int, string)> s7;
				signal<void(int, int, int, int)> s8;
				signal<void(int, int, int, string)> s9;
				signal<void(int, int, int, int, int)> s10;
				signal<void(int, int, int, int, string)> s11;

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
		};
	}
}
