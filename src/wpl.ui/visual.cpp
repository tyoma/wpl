#include <wpl/ui/visual.h>

namespace wpl
{
	namespace ui
	{
		gcontext::gcontext(surface_type &surface, renderer_type &renderer, const agge::rect_i &window)
			: _surface(surface), _renderer(renderer), _window(window)
		{	}

		gcontext gcontext::transform(int offset_x, int offset_y) const
		{
			agge::rect_i window2 = {
				_window.x1 - offset_x, _window.y1 - offset_y, _window.x2 - offset_x, _window.y2 - offset_y
			};
			return gcontext(_surface, _renderer, window2);
		}

		agge::rect_i gcontext::update_area() const
		{	return _window;	}


		void visual::draw(gcontext &/*ctx*/, gcontext::rasterizer_ptr &/*rasterizer*/) const
		{	}

		void visual::resize(unsigned /*cx*/, unsigned /*cy*/)
		{	}
	}
}
