#include <wpl/mt/thread.h>

#include <wpl/mt/synchronization.h>
#include <windows.h>
#include <functional>

namespace std
{
	using tr1::bind;
}

using namespace std;
using namespace System;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

namespace wpl
{
	namespace mt
	{
		namespace tests
		{
			namespace
			{
				void do_nothing()
				{	}

				void threadid_capture(unsigned int *thread_id, unsigned int wait_ms)
				{
					if (wait_ms > 0)
						::Sleep(wait_ms);
					*thread_id = ::GetCurrentThreadId();
				}

				template <typename T>
				void get_from_tls(const tls<T> *ptls, T **result)
				{	*result = ptls->get();	}
			}

			[TestClass]
			public ref class ThreadTests
			{
			public:
				[TestMethod]
				void ThreadCtorStartsNewThread()
				{
					// INIT
					unsigned int new_thread_id;
					{

						// ACT
						thread t(bind(&threadid_capture, &new_thread_id, 0));
					}

					// ASSERT
					Assert::AreNotEqual(::GetCurrentThreadId(), new_thread_id);
				}


				[TestMethod]
				void ThreadDtorWaitsForExecution()
				{
					// INIT
					unsigned int new_thread_id = ::GetCurrentThreadId();
					{

						// ACT
						thread t(bind(&threadid_capture, &new_thread_id, 100));

						// ASSERT
						Assert::AreEqual(::GetCurrentThreadId(), new_thread_id);

						// ACT
					}

					// ASSERT
					Assert::AreNotEqual(::GetCurrentThreadId(), new_thread_id);
				}


				[TestMethod]
				void ThreadIdIsNonZero()
				{
					// INIT
					thread t(&do_nothing);

					// ACT / ASSERT
					Assert::IsTrue(0 != t.id());
				}


				[TestMethod]
				void ThreadIdEqualsOSIdValue()
				{
					// INIT
					unsigned int new_thread_id = ::GetCurrentThreadId();

					// ACT
					unsigned int id = thread(bind(&threadid_capture, &new_thread_id, 0)).id();

					// ASSERT
					Assert::IsTrue(new_thread_id == id);
				}


				[TestMethod]
				void ThreadIdsAreUnique()
				{
					// INIT / ACT
					thread t1(&do_nothing), t2(&do_nothing), t3(&do_nothing);

					// ACT / ASSERT
					Assert::IsTrue(t1.id() != t2.id());
					Assert::IsTrue(t2.id() != t3.id());
					Assert::IsTrue(t3.id() != t1.id());
				}


				[TestMethod]
				void InitializedRunReturnsValidThreads()
				{
					// INIT / ACT
					auto_ptr<thread> t1(thread::run(&do_nothing, &do_nothing));
					auto_ptr<thread> t2(thread::run(&do_nothing, &do_nothing));

					// ASSERT
					Assert::IsTrue(t1.get() != 0);
					Assert::IsTrue(t2.get() != 0);
					Assert::IsTrue(t2->id() != t1->id());
				}


				[TestMethod]
				void InitializedRunInitializerAndJobAreCalledFromNewThread()
				{
					// INIT
					unsigned int id_initializer1, id_job1, id_initializer2, id_job2;

					// ACT
					auto_ptr<thread> t1(thread::run(bind(&threadid_capture, &id_initializer1, 0), bind(&threadid_capture, &id_job1, 0)));
					auto_ptr<thread> t2(thread::run(bind(&threadid_capture, &id_initializer2, 0), bind(&threadid_capture, &id_job2, 0)));

					::Sleep(100);

					// ASSERT
					Assert::IsTrue(t1->id() == id_initializer1);
					Assert::IsTrue(t1->id() == id_job1);
					Assert::IsTrue(t2->id() == id_initializer2);
					Assert::IsTrue(t2->id() == id_job2);
				}


				[TestMethod]
				void ThreadInitializerIsCalledSynchronuously()
				{
					// INIT
					unsigned int id_initializer1, id_initializer2;

					// ACT
					auto_ptr<thread> t1(thread::run(bind(&threadid_capture, &id_initializer1, 100), &do_nothing));
					auto_ptr<thread> t2(thread::run(bind(&threadid_capture, &id_initializer2, 100), &do_nothing));

					// ASSERT
					Assert::IsTrue(t1->id() == id_initializer1);
					Assert::IsTrue(t2->id() == id_initializer2);
				}


				[TestMethod]
				void TlsValueIsNullAfterInitialization()
				{
					// INIT / ACT
					tls<int> tls_int;
					tls<const double> tls_dbl;

					// ACT / ASSERT
					Assert::IsTrue(tls_int.get() == 0);
					Assert::IsTrue(tls_dbl.get() == 0);
				}


				[TestMethod]
				void TlsReturnsSameObjectInHostThread()
				{
					// INIT
					int a = 123;
					double b = 12.3;

					// ACT
					tls<int> tls_int;
					tls<const double> tls_dbl;

					tls_int.set(&a);
					tls_dbl.set(&b);

					int *ptr_a = tls_int.get();
					const double *ptr_b = tls_dbl.get();

					// ASSERT
					Assert::IsTrue(ptr_a == &a);
					Assert::IsTrue(ptr_b == &b);
				}


				[TestMethod]
				void TlsReturnsNullWhenValueGotFromAnotherThread()
				{
					// INIT
					tls<int> tls_int;
					tls<const double> tls_dbl;
					int a = 123;
					int *ptr_a = &a;
					double b = 12.3;
					const double *ptr_b = &b;

					tls_int.set(&a);
					tls_dbl.set(&b);

					// ACT
					thread(bind(&get_from_tls<int>, &tls_int, &ptr_a));
					thread(bind(&get_from_tls<const double>, &tls_dbl, &ptr_b));

					// ASSERT
					Assert::IsTrue(ptr_a == 0);
					Assert::IsTrue(ptr_b == 0);
				}


				[TestMethod]
				void TlsReturnsValueSetInSpecificThread()
				{
					// INIT
					tls<int> tls_int;
					tls<const double> tls_dbl;
					int a = 123;
					int a2 = 234;
					int *ptr_a = &a;
					double b = 12.3;
					double b2 = 23.4;
					const double *ptr_b = &b;

					tls_int.set(&a);
					tls_dbl.set(&b);

					// ACT
					thread::run(bind(&tls<int>::set, &tls_int, &a2), bind(&get_from_tls<int>, &tls_int, &ptr_a));
					thread::run(bind(&tls<const double>::set, &tls_dbl, &b2), bind(&get_from_tls<const double>, &tls_dbl, &ptr_b));

					// ASSERT
					Assert::IsTrue(ptr_a == &a2);
					Assert::IsTrue(ptr_b == &b2);
					Assert::IsTrue(tls_int.get() == &a);
					Assert::IsTrue(tls_dbl.get() == &b);
				}
			};
		}
	}
}
