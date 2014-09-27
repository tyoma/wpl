#pragma once

#include <wpl/base/concepts.h>

#include <algorithm>
#include <iterator>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace std
{
	using tr1::shared_ptr;
}

typedef struct HWND__ *HWND;	// stolen from windows.h
typedef struct tagRECT RECT;

namespace ut
{
	RECT get_window_rect(HWND hwnd);
	RECT rect(int left, int top, int width, int height);
	std::wstring get_window_text(HWND hwnd);


	[Microsoft::VisualStudio::TestTools::UnitTesting::TestClass]
	public ref class WindowTestsBase
	{
		std::vector<HWND> *_windows;
		std::vector< std::shared_ptr<wpl::destructible> > *_connections;

	protected:
		HWND create_window();
		HWND create_visible_window();
		HWND create_window(const std::wstring &class_name);
		HWND create_window(const std::wstring &class_name, HWND parent, unsigned int style, unsigned int stylex);
		void enable_reflection(HWND hwnd);

		void destroy_window(HWND hwnd);

	public:
		WindowTestsBase();
		~WindowTestsBase();

		[Microsoft::VisualStudio::TestTools::UnitTesting::TestCleanup]
		void cleanup();

		[Microsoft::VisualStudio::TestTools::UnitTesting::AssemblyInitialize]
		static void init_commctrl(Microsoft::VisualStudio::TestTools::UnitTesting::TestContext ^context);
	};

	class window_tracker
	{
		std::set<HWND> _windows;
		std::wstring _allow, _prohibit;

	public:
		window_tracker(const std::wstring &allow = L"", const std::wstring &prohibit = L"MSCTFIME UI");

		void checkpoint();

		std::vector<HWND> created;
		std::vector<HWND> destroyed;
	};

	template <typename T, size_t n>
	inline T *begin(T (&container)[n])
	{	return container;	}

	template <typename Container>
	inline typename Container::iterator begin(Container &container)
	{	return container.begin();	}

	template <typename T, size_t n>
	inline T *end(T (&container)[n])
	{	return container + n;	}

	template <typename Container>
	inline typename Container::iterator end(Container &container)
	{	return container.end();	}

	template <typename T, typename Container>
	inline void AreEquivalent(T &expected, const Container &actual)
	{
		using namespace std;

		const size_t n = distance(begin(expected), end(expected));

		if (actual.size() != n)
			Assert::Fail("Sequence contains {0} elements while {1} was expected!", actual.size(), n);

		vector<typename Container::value_type> expected_(begin(expected), end(expected));
		vector<typename Container::value_type> actual_(actual.begin(), actual.end());
		
		sort(expected_.begin(), expected_.end()), sort(actual_.begin(), actual_.end());

		const size_t mismatched = distance(expected_.begin(), mismatch(expected_.begin(), expected_.end(), actual_.begin()).first);

		if (mismatched != n)
			Assert::Fail("Sequences are different starting from position #{0}!", mismatched);
	}

	template <typename T, typename Container>
	inline void AreEqual(T &expected, const Container &actual)
	{
		using namespace std;

		const size_t n = distance(begin(expected), end(expected));

		if (actual.size() != n)
			Assert::Fail("Sequence contains {0} elements while {1} was expected!", actual.size(), n);

		const size_t mismatched = distance(begin(expected), mismatch(begin(expected), end(expected), begin(actual)).first);

		if (mismatched != n)
			Assert::Fail("Sequences are different starting from position #{0}!", mismatched);
	}
}

bool operator ==(const RECT &lhs, const RECT &rhs);

#define ASSERT_THROWS(fragment, expected_exception) try { fragment; Assert::Fail("Expected exception was not thrown!"); } catch (const expected_exception &) { } catch (AssertFailedException ^) { throw; } catch (...) { Assert::Fail("Exception of unexpected type was thrown!"); }
