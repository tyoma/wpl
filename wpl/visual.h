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

#pragma once

#include "signals.h"
#include "types.h"

#include <agge/bitmap.h>
#include <agge/clipper.h>
#include <agge/platform/bitmap.h>
#include <agge/rasterizer.h>
#include <agge/renderer_parallel.h>
#include <string>

namespace agge
{
	template <typename RasterizerT>
	class text_engine;
}

namespace wpl
{
	struct cursor_manager;
	class native_view;

	class gcontext
	{
	public:
		typedef agge::pixel32 pixel_type;
		typedef agge::bitmap<pixel_type, agge::platform::raw_bitmap> surface_type;
		typedef agge::rasterizer< agge::clipper<int> > rasterizer_type;
		typedef std::auto_ptr<rasterizer_type> rasterizer_ptr;
		typedef agge::renderer_parallel renderer_type;
		typedef agge::text_engine<rasterizer_type> text_engine_type;

	public:
		gcontext(surface_type &surface, renderer_type &renderer, text_engine_type &text_engine_,
			const agge::vector_i &offset, const agge::rect_i *window_ = 0) throw();

		gcontext translate(int offset_x, int offset_y) const throw();
		gcontext window(int x1, int y1, int x2, int y2) const throw();

		agge::rect_i update_area() const throw();

		template <typename BlenderT, typename AlphaFn>
		void operator ()(rasterizer_ptr &rasterizer, const BlenderT &blender, const AlphaFn &alpha);

	public:
		text_engine_type &text_engine;

	private:
		const gcontext &operator =(const gcontext &rhs);

	private:
		surface_type &_surface;
		renderer_type &_renderer;
		const agge::vector_i _offset;
		const agge::rect_i _window;
	};


	struct font
	{
		std::wstring typeface;
		int height; // em height in points.
	};


	struct visual
	{
		struct positioned_native_view;
		typedef std::vector<positioned_native_view> positioned_native_views;

		visual();
		virtual ~visual();

		virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const;
		virtual void resize(unsigned cx, unsigned cy, positioned_native_views &native_views);

		bool transcending;

		signal<void(const agge::rect_i *window)> invalidate;
		signal<void()> force_layout;
	};

	struct visual::positioned_native_view
	{
		positioned_native_view(native_view &nview_, const view_location &location_) throw();

		native_view &get_view() const throw();

		view_location location;

	private:
		native_view *_nview;
	};



	template <typename BlenderT, typename AlphaFn>
	inline void gcontext::operator ()(rasterizer_ptr &rasterizer, const BlenderT &blender, const AlphaFn &alpha)
	{
		rasterizer->sort();
		_renderer(_surface, _offset, &_window, *rasterizer, blender, alpha);
		rasterizer->reset();
	}
}
