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

#include <wpl/visual_router.h>

#include <wpl/helpers.h>
#include <wpl/view.h>

using namespace std;

namespace wpl
{
	visual_router::visual_router(const vector<placed_view> &views, visual_router_host &host)
		: _views(views), _host(host)
	{	}

	void visual_router::reload_views()
	{
		size_t index = 0;

		_connections.clear();
		for (auto i = _views.begin(); i != _views.end(); ++i, ++index)
		{
			if (const auto v = i->regular)
			{
				_connections.push_back(v->invalidate += [this, index] (const agge::rect_i *area) {
					if (index < _views.size())
					{
						const auto &l = _views[index].location;

						if (area)
						{
							auto a = *area;

							offset(a, l.x1, l.y1);
							_host.invalidate(a);
						}
						else
						{
							_host.invalidate(l);
						}
					}
				});
			}
		}
	}

	void visual_router::draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const
	{
		for (auto i = _views.begin(); i != _views.end(); ++i)
		{
			if (const auto v = i->regular)
			{
				auto child_ctx = ctx.translate(i->location.x1, i->location.y1);

				if (v->transcending)
				{
					v->draw(child_ctx, rasterizer);
				}
				else
				{
					auto child_ctx_windowed = child_ctx.window(0, 0, wpl::width(i->location), wpl::height(i->location));

					v->draw(child_ctx_windowed, rasterizer);
				}
			}
		}
	}
}
