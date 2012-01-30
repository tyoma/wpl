#include <wpl/ui/win32/containers.h>
#include <wpl/ui/form.h>

#include "TestHelpers.h"

using namespace Microsoft::VisualStudio::TestTools::UnitTesting;
using namespace std;

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			[TestClass]
			public ref class FormTests : ut::WindowTestsBase
			{
			public:
				[TestInitialize]
				void create_dummy_window()
				{
					create_window();
				}


				[TestMethod]
				void FormWindowIsCreatedAtFormConstruction()
				{
					// INIT
					set<void *> before(ut::enum_thread_windows());

					// ACT
					shared_ptr<form> f1 = create_form();

					// ASSERT
					set<void *> after(ut::enum_thread_windows());

					Assert::IsTrue(1 == ut::added_items(before, after));
					Assert::IsTrue(0 == ut::removed_items(before, after));

					// ACT
					shared_ptr<form> f2 = create_form();
					shared_ptr<form> f3 = create_form();

					// ASSERT
					set<void *> after2(ut::enum_thread_windows());

					Assert::IsTrue(2 == ut::added_items(after, after2));
					Assert::IsTrue(3 == ut::added_items(before, after2));
					Assert::IsTrue(0 == ut::removed_items(before, after2));
				}


				[TestMethod]
				void FormConstructionReturnsNonNullObject()
				{
					// INIT / ACT / ASSERT
					Assert::IsTrue(!!create_form());
				}


				[TestMethod]
				void FormDestructionDestroysItsWindow()
				{
					// INIT
					shared_ptr<form> f1 = create_form();
					shared_ptr<form> f2 = create_form();
					set<void *> before(ut::enum_thread_windows());

					// ACT
					f1 = shared_ptr<form>();
					set<void *> after1(ut::enum_thread_windows());
					f2 = shared_ptr<form>();
					set<void *> after2(ut::enum_thread_windows());

					// ASSERT
					Assert::IsTrue(1 == ut::removed_items(before, after1));
					Assert::IsTrue(1 == ut::removed_items(after1, after2));
					Assert::IsTrue(2 == ut::removed_items(before, after2));
					Assert::IsTrue(0 == ut::added_items(before, after2));
				}
			};
		}
	}
}
