#pragma once

#include <string>
#include <vector>
#include <tchar.h>

namespace ut
{
	std::wstring make_native(System::String ^managed_string);
	System::String ^make_managed(const std::wstring &native_string);

	[Microsoft::VisualStudio::TestTools::UnitTesting::TestClass]
	public ref class WindowTestsBase
	{
		std::vector<void *> *_windows;

	protected:
		void *create_window();
		void *create_window(const TCHAR *class_name);
		void *create_window(const TCHAR *class_name, void *parent, unsigned int style, unsigned int stylex);

		void destroy_window(void *hwnd);

	public:
		WindowTestsBase();
		~WindowTestsBase();

		[Microsoft::VisualStudio::TestTools::UnitTesting::TestCleanup]
		void cleanup();

		[Microsoft::VisualStudio::TestTools::UnitTesting::AssemblyInitialize]
		static void init_commctrl(Microsoft::VisualStudio::TestTools::UnitTesting::TestContext ^context);
	};
}

#define ASSERT_THROWS(fragment, expected_exception) try { fragment; Assert::Fail("Expected exception was not thrown!"); } catch (const expected_exception &) { } catch (AssertFailedException ^) { throw; } catch (...) { Assert::Fail("Exception of unexpected type was thrown!"); }
