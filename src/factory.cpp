#include <wpl/factory.h>

#include <stdexcept>

using namespace std;

namespace wpl
{
	factory::factory(const shared_ptr<gcontext::surface_type> &backbuffer,
			const shared_ptr<gcontext::renderer_type> &renderer,
			const std::shared_ptr<gcontext::text_engine_type> &text_engine, const shared_ptr<stylesheet> &stylesheet_)
		: _backbuffer(backbuffer), _renderer(renderer), _text_engine(text_engine), _stylesheet(stylesheet_)
	{	}

	void factory::register_form(const form_constructor &constructor)
	{	_default_form_constructor = constructor;	}

	void factory::register_control(const char *type, const control_constructor &constructor)
	{	_control_constructors[type] = constructor;	}

	shared_ptr<form> factory::create_form() const
	{
		if (_default_form_constructor)
			return _default_form_constructor(_backbuffer, _renderer, _text_engine, _stylesheet);
		throw invalid_argument("");
	}

	shared_ptr<control> factory::create_control(const char *type) const
	{
		auto i = _control_constructors.find(type);

		if (i != _control_constructors.end())
			return i->second(*this, _stylesheet);
		throw invalid_argument("");
	}

	shared_ptr<factory> factory::create_default(const shared_ptr<gcontext::surface_type> &backbuffer,
		const shared_ptr<gcontext::renderer_type> &renderer,
		const std::shared_ptr<gcontext::text_engine_type> &text_engine, const shared_ptr<stylesheet> &stylesheet_)
	{
		shared_ptr<factory> f(new factory(backbuffer, renderer, text_engine, stylesheet_));

		setup_default(*f);
		return f;
	}
}
