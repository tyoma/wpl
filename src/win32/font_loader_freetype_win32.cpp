//	Copyright (c) 2011-2020 by Artem A. Gevorkyan (gevorkyan.org)
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.

#include <wpl/freetype2/font_loader.h>

#include <regex>
#include <windows.h>

using namespace std;

namespace wpl
{
	namespace
	{
		const auto c_default_font_directory = L"C:\\Windows\\Fonts";
		const regex c_font_match(".+\\.(ttf|otf|ttc)", regex::icase);

		wstring append_path(wstring lhs, const wstring &rhs)
		{
			if (!lhs.empty() && lhs.back() != L'\\' && lhs.back() != L'/')
				lhs += L"\\";
			lhs += rhs;
			return lhs;
		}
	}

	class directory_enumerator : noncopyable
	{
	public:
		directory_enumerator(const wstring &directory)
			: _directory(directory), _handle(NULL)
		{	}

		~directory_enumerator()
		{
			if (_handle)
				::FindClose(_handle);
		}

		bool operator ()(string &path)
		{
			WIN32_FIND_DATAW fd = {};

			if (_handle)
			{
				if (!::FindNextFileW(_handle, &fd))
					return false;
			}
			else
			{
				const auto handle = ::FindFirstFileW(append_path(_directory, L"*").c_str(), &fd);

				if (INVALID_HANDLE_VALUE == handle)
					return false;
				_handle = handle;
			}
			_filepath = append_path(_directory, fd.cFileName);
			path.assign(_filepath.begin(), _filepath.end());
			return true;
		}

	private:
		const wstring _directory;
		wstring _filepath;
		HANDLE _handle;
	};

	font_loader::enum_font_files_cb font_loader::create_fonts_enumerator()
	{
		auto e = make_shared<directory_enumerator>(c_default_font_directory);

		return [e] (string &path) -> bool {
			while ((*e)(path))
			{
				if (regex_match(path, c_font_match))
					return true;
			}
			return false;
		};
	}
}
