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

#include <wpl/macos/cursor_manager.h>

#include <Cocoa/Cocoa.h>

using namespace std;

namespace wpl
{
	namespace macos
	{
		cursor_manager::cursor_manager()
			: _cursors(new cached_cursors)
		{	}

		cursor_manager::~cursor_manager()
		{	}

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
				[static_cast<NSCursor *>(i->second.get()) set];
		}

		void cursor_manager::push(std::shared_ptr<const cursor> cursor_)
		{
			auto i = _cursors->find(cursor_.get());

			if (_cursors->end() != i)
				[static_cast<NSCursor *>(i->second.get()) push];
		}

		void cursor_manager::pop()
		{
			[NSCursor pop];
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
			NSCursor *cursor_;

			switch (id_)
			{
			case cursor_manager::arrow:	cursor_ = [NSCursor arrowCursor];	break;
			case cursor_manager::i_beam:	cursor_ = [NSCursor IBeamCursor];	break;
			case cursor_manager::hand:	cursor_ = [NSCursor pointingHandCursor];	break;
			case cursor_manager::crosshair:	cursor_ = [NSCursor crosshairCursor];	break;
			case cursor_manager::h_resize:	cursor_ = [NSCursor resizeLeftRightCursor];	break;
			case cursor_manager::l_resize:	cursor_ = [NSCursor resizeLeftCursor];	break;
			case cursor_manager::r_resize:	cursor_ = [NSCursor resizeRightCursor];	break;
			case cursor_manager::v_resize:	cursor_ = [NSCursor resizeUpDownCursor];	break;
			case cursor_manager::t_resize:	cursor_ = [NSCursor resizeUpCursor];	break;
			case cursor_manager::b_resize:	cursor_ = [NSCursor resizeDownCursor];	break;
			default:	return shared_ptr<void>();
			}
			return shared_ptr<void>([cursor_ retain], [] (NSCursor *p) {	[p release];	});
		}
	}
}
