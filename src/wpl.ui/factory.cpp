#include <wpl/factory.h>

#include <stdexcept>

using namespace std;

namespace wpl
{
	factory::factory(const std::shared_ptr<stylesheet> &stylesheet_)
		: _renderer(new gcontext::renderer_type(2)), _backbuffer(new gcontext::surface_type(100, 100)),
			_stylesheet(stylesheet_)
	{	}

	void factory::register_form(const form_constructor &constructor)
	{	_default_form_constructor = constructor;	}

	void factory::register_control(const char *type, const control_constructor &constructor)
	{	_control_constructors[type] = constructor;	}

	shared_ptr<form> factory::create_form() const
	{
		if (_default_form_constructor)
			return _default_form_constructor(_renderer, _backbuffer, _stylesheet);
		throw invalid_argument("");
	}

	shared_ptr<control> factory::create_control(const char *type) const
	{
		auto i = _control_constructors.find(type);

		if (i != _control_constructors.end())
			return i->second(*this, _stylesheet);
		throw invalid_argument("");
	}
}
