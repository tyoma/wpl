#include <wpl/mt/thread.h>

#include <functional>
#include <ut/assert.h>
#include <ut/test.h>
#include <wpl/mt/synchronization.h>
#include <windows.h>

using namespace std;

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

				void wait(void *event)
				{	::WaitForSingleObject(event, INFINITE);	}

				template <typename T>
				void get_from_tls(const tls<T> *ptls, T **result)
				{	*result = ptls->get();	}
			}

			begin_test_suite( ThreadTests )
				test( ThreadCtorStartsNewThread )
				{
					// INIT
					unsigned int new_thread_id;
					{

						// ACT
						thread t(bind(&threadid_capture, &new_thread_id, 0));
					}

					// ASSERT
					assert_not_equal(::GetCurrentThreadId(), new_thread_id);
				}


				test( ThreadDtorWaitsForExecution )
				{
					// INIT
					unsigned int new_thread_id = ::GetCurrentThreadId();
					{

						// ACT
						thread t(bind(&threadid_capture, &new_thread_id, 100));

						// ASSERT
						assert_equal(::GetCurrentThreadId(), new_thread_id);

						// ACT
					}

					// ASSERT
					assert_not_equal(::GetCurrentThreadId(), new_thread_id);
				}


				test( ThreadIdIsNonZero )
				{
					// INIT
					thread t(&do_nothing);

					// ACT / ASSERT
					assert_not_equal(0u, t.get_id());
				}


				test( ThreadIdEqualsOSIdValue )
				{
					// INIT
					unsigned int new_thread_id = ::GetCurrentThreadId();

					// ACT
					unsigned int id = thread(bind(&threadid_capture, &new_thread_id, 0)).get_id();

					// ASSERT
					assert_equal(new_thread_id, id);
				}


				test( ThreadIdsAreUnique )
				{
					// INIT / ACT
					thread t1(&do_nothing), t2(&do_nothing), t3(&do_nothing);

					// ACT / ASSERT
					assert_not_equal(t1.get_id(), t2.get_id());
					assert_not_equal(t2.get_id(), t3.get_id());
					assert_not_equal(t3.get_id(), t1.get_id());
				}


				test( InitializedRunReturnsValidThreads )
				{
					// INIT / ACT
					auto_ptr<thread> t1(thread::run(&do_nothing, &do_nothing));
					auto_ptr<thread> t2(thread::run(&do_nothing, &do_nothing));

					// ASSERT
					assert_not_null(t1.get());
					assert_not_null(t2.get());
					assert_not_equal(t2->get_id(), t1->get_id());
				}


				test( DetachedThreadContinuesExecution )
				{
					// INIT
					shared_ptr<void> e(::CreateEvent(NULL, TRUE, FALSE, NULL), &::CloseHandle);
					auto_ptr<thread> t(new thread(bind(&wait, e.get())));
					unsigned int thread_id = t->get_id();
					DWORD exit_code = 0;
					shared_ptr<void> hthread(::OpenThread(THREAD_QUERY_INFORMATION | SYNCHRONIZE, FALSE, thread_id),
						&::CloseHandle);

					// ACT
					t->detach();
					t.reset();

					// ASSERT
					::GetExitCodeThread(hthread.get(), &exit_code);
					
					assert_equal(STILL_ACTIVE, exit_code);

					// ACT
					::SetEvent(e.get());
					wait(hthread.get());

					// ASSERT
					::GetExitCodeThread(hthread.get(), &exit_code);
					
					assert_equal(0u, exit_code);
				}


				test( JoinThreadGuaranteesItsCompletion )
				{
					// INIT
					auto_ptr<thread> t(new thread(&do_nothing));
					unsigned int thread_id = t->get_id();
					DWORD exit_code = STILL_ACTIVE;
					shared_ptr<void> hthread(::OpenThread(THREAD_QUERY_INFORMATION | SYNCHRONIZE, FALSE, thread_id),
						&::CloseHandle);

					// ACT
					t->join();

					// ASSERT
					::GetExitCodeThread(hthread.get(), &exit_code);
					
					assert_equal(0u, exit_code);
				}


				test( InitializedRunInitializerAndJobAreCalledFromNewThread )
				{
					// INIT
					unsigned int id_initializer1, id_job1, id_initializer2, id_job2;

					// ACT
					auto_ptr<thread> t1(thread::run(bind(&threadid_capture, &id_initializer1, 0), bind(&threadid_capture, &id_job1, 0)));
					auto_ptr<thread> t2(thread::run(bind(&threadid_capture, &id_initializer2, 0), bind(&threadid_capture, &id_job2, 0)));

					::Sleep(100);

					// ASSERT
					assert_equal(t1->get_id(), id_initializer1);
					assert_equal(t1->get_id(), id_job1);
					assert_equal(t2->get_id(), id_initializer2);
					assert_equal(t2->get_id(), id_job2);
				}


				test( ThreadInitializerIsCalledSynchronuously )
				{
					// INIT
					unsigned int id_initializer1, id_initializer2;

					// ACT
					auto_ptr<thread> t1(thread::run(bind(&threadid_capture, &id_initializer1, 100), &do_nothing));
					auto_ptr<thread> t2(thread::run(bind(&threadid_capture, &id_initializer2, 100), &do_nothing));

					// ASSERT
					assert_equal(t1->get_id(), id_initializer1);
					assert_equal(t2->get_id(), id_initializer2);
				}


				test( TlsValueIsNullAfterInitialization )
				{
					// INIT / ACT
					tls<int> tls_int;
					tls<const double> tls_dbl;

					// ACT / ASSERT
					assert_null(tls_int.get());
					assert_null(tls_dbl.get());
				}


				test( TlsReturnsSameObjectInHostThread )
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
					assert_equal(ptr_a, &a);
					assert_equal(ptr_b, &b);
				}


				test( TlsReturnsNullWhenValueGotFromAnotherThread )
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
					assert_null(ptr_a);
					assert_null(ptr_b);
				}


				test( TlsReturnsValueSetInSpecificThread )
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
					assert_equal(ptr_a, &a2);
					assert_equal(ptr_b, &b2);
					assert_equal(tls_int.get(), &a);
					assert_equal(tls_dbl.get(), &b);
				}
			end_test_suite
		}
	}
}
