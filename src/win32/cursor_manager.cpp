//	Copyright (c) 2011-2022 by Artem A. Gevorkyan (gevorkyan.org)
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

#include <wpl/win32/cursor_manager.h>

#include <windows.h>

using namespace std;

namespace wpl
{
	namespace win32
	{
		cursor_manager::cursor_manager()
			: _cursors(new cached_cursors)
		{	}

		cursor_manager::~cursor_manager()
		{
			if (!_cursor_stack.empty())
				::SetCursor(static_cast<HCURSOR>(_cursor_stack.front()));
		}

		shared_ptr<const cursor> cursor_manager::get(standard_cursor id) const
		{
			auto i = _standard_cursors.find(id);

			if (_standard_cursors.end() == i)
			{
				auto c = associate(get_cursor(id));

				i = _standard_cursors.insert(make_pair(id, c)).first;
			}
			return i->second;
		}

		void cursor_manager::set(shared_ptr<const cursor> cursor_)
		{
			auto i = _cursors->find(cursor_.get());

			if (_cursors->end() != i)
				::SetCursor(static_cast<HCURSOR>(i->second.get()));
		}

		void cursor_manager::push(std::shared_ptr<const cursor> cursor_)
		{
			_cursor_stack.push_back(::GetCursor());
			set(cursor_);
		}

		void cursor_manager::pop()
		{
			if (_cursor_stack.empty())
				return;
			::SetCursor(static_cast<HCURSOR>(_cursor_stack.back()));
			_cursor_stack.pop_back();
		}

		shared_ptr<const cursor> cursor_manager::associate(shared_ptr<void> hcursor) const
		{
			const auto m = _cursors;
			unique_ptr<cursor> c(new cursor(1, 1, 0, 0));
			auto i = m->insert(make_pair(c.get(), hcursor)).first;

			return shared_ptr<const cursor>(c.release(), [m, i] (const cursor *p) {
				m->erase(i);
				delete p;
			});
		}

		shared_ptr<void> cursor_manager::get_cursor(cursor_manager::standard_cursor id_)
		{
			LPCTSTR id;

			switch (id_)
			{
			case cursor_manager::arrow:	id = IDC_ARROW;	break;
			case cursor_manager::i_beam:	id = IDC_IBEAM;	break;
			case cursor_manager::hand:	id = IDC_HAND;	break;
			case cursor_manager::crosshair:	id = IDC_CROSS;	break;
			case cursor_manager::h_resize:	id = IDC_SIZEWE;	break;
			case cursor_manager::l_resize:	id = IDC_SIZEWE;	break;
			case cursor_manager::r_resize:	id = IDC_SIZEWE;	break;
			case cursor_manager::v_resize:	id = IDC_SIZENS;	break;
			case cursor_manager::t_resize:	id = IDC_SIZENS;	break;
			case cursor_manager::b_resize:	id = IDC_SIZENS;	break;
			default:	return shared_ptr<void>();
			}
			return shared_ptr<void>(static_cast<HCURSOR>(::LoadImage(0, id, IMAGE_CURSOR, 0, 0, LR_SHARED)),
				&::DestroyCursor);
		}
	}
}
