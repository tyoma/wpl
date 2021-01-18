//	Copyright (c) 2011-2021 by Artem A. Gevorkyan (gevorkyan.org)
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

#pragma once

#include "../factory_context.h"
#include "../visual_router.h"

#include <windows.h>

namespace wpl
{
	namespace win32
	{
		class visual_router : visual_router_host
		{
		public:
			visual_router(HWND hwnd, const std::vector<placed_view> &views, const form_context &context);

			void reload_views();
			void set_offset(const agge::agge_vector<int> &offset);
			bool handle_message(LRESULT &result, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

		private:
			// visual_router_host methods
			virtual void invalidate(const agge::rect_i &area) override;

		private:
			HWND _hwnd;
			wpl::visual_router _underlying;
			const form_context _context;
			gcontext::rasterizer_ptr _rasterizer;
			agge::agge_vector<int> _offset;
		};
	}
}
