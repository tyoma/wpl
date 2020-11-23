#include <wpl/stylesheet_db.h>

#include <algorithm>
#include <stdexcept>

using namespace agge;
using namespace std;

namespace wpl
{
	void stylesheet_db::set_color(const char *id, agge::color value)
	{	_colors[id] = value;	}

	void stylesheet_db::set_font(const char *id, agge::font::ptr value)
	{	_fonts[id] = value;	}

	void stylesheet_db::set_value(const char *id, agge::real_t value)
	{	_values[id] = value;	}

	color stylesheet_db::get_color(const char *id) const
	{	return get_value(_colors, id);	}

	font::ptr stylesheet_db::get_font(const char *id) const
	{	return get_value(_fonts, id);	}

	real_t stylesheet_db::get_value(const char *id) const
	{	return get_value(_values, id);	}

	template <typename ContainerT>
	typename ContainerT::mapped_type stylesheet_db::get_value(const ContainerT &container_, const char *id) const
	{
		size_t pos;
		string partial_id;

		do
		{
			typename ContainerT::const_iterator i = container_.find(id);

			if (i != container_.end())
				return i->second;
			if (partial_id.empty())
				partial_id = id;
			pos = partial_id.find_last_of('.');
			if (string::npos != pos)
				partial_id.resize(pos), id = partial_id.c_str();
		} while (string::npos != pos);
		throw invalid_argument(id);
	}
}
