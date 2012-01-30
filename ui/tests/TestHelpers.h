#pragma once

#include <wpl/base/concepts.h>

#include <string>
#include <set>
#include <vector>
#include <algorithm>
#include <memory>
#include <iterator>
#include <tchar.h>

namespace std
{
	using tr1::shared_ptr;
}

namespace ut
{
	std::wstring make_native(System::String ^managed_string);
	System::String ^make_managed(const std::wstring &native_string);
	std::set<void *> enum_thread_windows();

	[Microsoft::VisualStudio::TestTools::UnitTesting::TestClass]
	public ref class WindowTestsBase
	{
		std::vector<void *> *_windows;
		std::vector< std::shared_ptr<wpl::destructible> > *_connections;

	protected:
		void *create_window();
		void *create_visible_window();
		void *create_window(const TCHAR *class_name);
		void *create_window(const TCHAR *class_name, void *parent, unsigned int style, unsigned int stylex);
		void enable_reflection(void *hwnd);

		void destroy_window(void *hwnd);

	public:
		WindowTestsBase();
		~WindowTestsBase();

		[Microsoft::VisualStudio::TestTools::UnitTesting::TestCleanup]
		void cleanup();

		[Microsoft::VisualStudio::TestTools::UnitTesting::AssemblyInitialize]
		static void init_commctrl(Microsoft::VisualStudio::TestTools::UnitTesting::TestContext ^context);
	};

	class null_output_iterator : public std::iterator<std::output_iterator_tag, null_output_iterator>
	{
		size_t *_count;

	public:
		null_output_iterator(size_t &count)
			: _count(&count)
		{	}

		template <typename T>
		void operator =(T const &)
		{	}

		null_output_iterator &operator++()
		{
			++*_count;
			return *this;
		}

		null_output_iterator operator++(int)
		{
			null_output_iterator it(*this);

			++*this;
			return it;
		}

		null_output_iterator &operator *()
		{	return *this;	}
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
		size_t count = 0;

		std::set_difference(after.begin(), after.end(), before.begin(), before.end(), null_output_iterator(count));
		return count;
	}
	
	template <typename C1, typename C2>
	inline size_t removed_items(const C1 &before, const C2 &after)
	{	return added_items(after, before);	}
}

#define ASSERT_THROWS(fragment, expected_exception) try { fragment; Assert::Fail("Expected exception was not thrown!"); } catch (const expected_exception &) { } catch (AssertFailedException ^) { throw; } catch (...) { Assert::Fail("Exception of unexpected type was thrown!"); }
