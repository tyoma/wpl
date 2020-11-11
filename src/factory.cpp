#include <wpl/factory.h>

#include <stdexcept>

using namespace std;

namespace wpl
{
	factory::factory(const form_context &context)
		: _context(context)
	{	}

	void factory::register_form(const form_constructor &constructor)
	{	_default_form_constructor = constructor;	}

	void factory::register_control(const char *type, const control_constructor &constructor)
	{	_control_constructors[type] = constructor;	}

	shared_ptr<form> factory::create_form() const
	{
		if (_default_form_constructor)
			return _default_form_constructor(_context);
		throw invalid_argument("");
	}

	shared_ptr<control> factory::create_control(const char *type) const
	{
		auto i = _control_constructors.find(type);
		control_context context = {	_context.stylesheet_, _context.clock_, _context.queue_,	};

		if (i != _control_constructors.end())
			return i->second(*this, context);
		throw invalid_argument("");
	}

	shared_ptr<factory> factory::create_default(const form_context &context)
	{
		shared_ptr<factory> f(new factory(context));

		setup_default(*f);
		return f;
	}
}
