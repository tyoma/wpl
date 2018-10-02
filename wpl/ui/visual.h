#pragma once

#include "../base/signals.h"

#include <agge/bitmap.h>
#include <agge/clipper.h>
#include <agge/platform/bitmap.h>
#include <agge/rasterizer.h>
#include <agge/renderer_parallel.h>

namespace wpl
{
	namespace ui
	{
		class gcontext
		{
		public:
			typedef agge::pixel32 pixel_type;
			typedef agge::bitmap<pixel_type, agge::platform::raw_bitmap> surface_type;
			typedef agge::rasterizer< agge::clipper<int> > rasterizer_type;
			typedef std::auto_ptr<rasterizer_type> rasterizer_ptr;
			typedef agge::renderer_parallel renderer_type;

		public:
			gcontext(surface_type &surface, renderer_type &renderer, const agge::rect_i &window);

			gcontext transform(int offset_x, int offset_y) const;

			agge::rect_i update_area() const;

			template <typename BlenderT, typename AlphaFn>
			void operator ()(rasterizer_ptr &rasterizer, const BlenderT &blender, const AlphaFn &alpha);

		private:
			const gcontext &operator =(const gcontext &rhs);

		private:
			surface_type &_surface;
			renderer_type &_renderer;
			const agge::rect_i _window;
		};

		struct visual
		{
			virtual ~visual() {	}

			virtual void draw(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer) const;
			virtual void resize(unsigned cx, unsigned cy);

			signal<void(const agge::rect_i *window)> invalidate;
		};



		template <typename BlenderT, typename AlphaFn>
		inline void gcontext::operator ()(rasterizer_ptr &rasterizer, const BlenderT &blender, const AlphaFn &alpha)
		{
			rasterizer->sort();
			_renderer(_surface, &_window, *rasterizer, blender, alpha);
			rasterizer->reset();
		}
	}
}
