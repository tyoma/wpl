#include <wpl/factory.h>

#include <stdexcept>

using namespace std;

namespace wpl
{
	factory::factory(const form_context &context_)
		: context(context_)
	{	}

	void factory::register_form(const form_constructor &constructor)
	{	_default_form_constructor = constructor;	}

	void factory::register_control(const char *type, const control_constructor &constructor)
	{	_control_constructors[type] = constructor;	}

	shared_ptr<form> factory::create_form() const
	{
		if (_default_form_constructor)
			return _default_form_constructor(context);
		throw invalid_argument("");
	}

	shared_ptr<control> factory::create_control(const char *type) const
	{
		auto i = _control_constructors.find(type);
		control_context context_ = {
			context.text_engine,
			context.stylesheet_,
			context.cursor_manager_,
			context.clock_,
			context.queue_,
		};

		if (i != _control_constructors.end())
			return i->second(*this, context_);
		throw invalid_argument("");
	}
}
