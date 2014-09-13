#pragma once

#include <wpl/base/concepts.h>

#include <string>
#include <set>
#include <vector>
#include <algorithm>
#include <memory>
#include <iterator>

namespace std
{
	using tr1::shared_ptr;
}

typedef struct HWND__ *HWND;	// stolen from windows.h
typedef struct tagRECT RECT;

namespace ut
{
	std::wstring make_native(System::String ^managed_string);
	System::String ^make_managed(const std::wstring &native_string);
	RECT get_window_rect(HWND hwnd);
	RECT rect(int left, int top, int width, int height);


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

	template <typename T, typename Container, size_t n>
	inline void AreEquivalent(T (&expected)[n], const Container &actual)
	{
		using namespace std;

		if (actual.size() != n)
			Assert::Fail("Sequence contains {0} elements while {1} was expected!", actual.size(), n);
		vector<T> expected_(expected, expected + n), actual_(actual.begin(), actual.end());
		sort(expected_.begin(), expected_.end()), sort(actual_.begin(), actual_.end());

		size_t mismatched = distance(expected_.begin(), mismatch(expected_.begin(), expected_.end(), actual_.begin()).first);

		if (mismatched != n)
			Assert::Fail("Sequences are different starting from position #{0}!", mismatched);
	}
	
	template <typename C1, typename C2>
	inline size_t added_items(const C1 &before, const C2 &after)
	{
		class counting_iterator : public std::iterator<std::output_iterator_tag, counting_iterator>
		{
			size_t *_count;

		public:
			counting_iterator(size_t &count)
				: _count(&count)
			{	}

			void operator =(typename C2::value_type const &)
			{	}

			counting_iterator &operator++()
			{
				++*_count;
				return *this;
			}

			counting_iterator operator++(int)
			{
				counting_iterator it(*this);

				++*this;
				return it;
			}

			counting_iterator &operator *()
			{	return *this;	}
		};

		size_t count = 0;

		std::set_difference(after.begin(), after.end(), before.begin(), before.end(), counting_iterator(count));
		return count;
	}
	
	template <typename C1, typename C2>
	inline size_t removed_items(const C1 &before, const C2 &after)
	{	return added_items(after, before);	}
}

bool operator ==(const RECT &lhs, const RECT &rhs);

#define ASSERT_THROWS(fragment, expected_exception) try { fragment; Assert::Fail("Expected exception was not thrown!"); } catch (const expected_exception &) { } catch (AssertFailedException ^) { throw; } catch (...) { Assert::Fail("Exception of unexpected type was thrown!"); }
