#include <wpl/visual.h>

using namespace agge;
using namespace std;

namespace agge
{
	template <typename T>
	inline rect<T> operator -(rect<T> lhs, const agge_vector<T> &rhs)
	{	return lhs.x1 -= rhs.dx, lhs.y1 -= rhs.dy, lhs.x2 -= rhs.dx, lhs.y2 -= rhs.dy, lhs;	}

	template <typename T>
	inline agge_vector<T> operator -(agge_vector<T> lhs, const agge_vector<T> &rhs)
	{	return lhs.dx -= rhs.dx, lhs.dy -= rhs.dy, lhs;	}
}

namespace wpl
{
	namespace
	{
		rect_i make_rect(int x1, int y1, int x2, int y2)
		{
			rect_i r = { x1, y1, x2, y2 };
			return r;
		}
	}

	gcontext::gcontext(surface_type &surface, renderer_type &renderer_, text_engine_type &text_engine_,
			const vector_i &offset, const rect_i *window_) throw()
		: text_engine(text_engine_), _surface(surface), _renderer(renderer_), _offset(offset),
			_window(window_ ? *window_ : make_rect(0, 0, surface.width(), surface.height()) - offset)
	{	}

	gcontext gcontext::translate(int offset_x, int offset_y) const throw()
	{
		const vector_i offset = { offset_x, offset_y };
		const rect_i w = _window - offset;

		return gcontext(_surface, _renderer, text_engine, _offset - offset, &w);
	}

	gcontext gcontext::window(int x1, int y1, int x2, int y2) const throw()
	{
		rect_i w = { x1, y1, x2, y2 };

		return gcontext(_surface, _renderer, text_engine, _offset, &w);
	}

	rect_i gcontext::update_area() const throw()
	{	return _window;	}


	visual::visual()
		: transcending(false)
	{	}

	void visual::draw(gcontext &/*ctx*/, gcontext::rasterizer_ptr &/*rasterizer*/) const
	{	}
}
