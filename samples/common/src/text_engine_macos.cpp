#include "../text_engine.h"

#include <wpl/freetype2/font_loader.h>

using namespace std;

namespace wpl
{
	std::shared_ptr<gcontext::text_engine_type> create_text_engine()
	{
		struct text_engine_composite
		{
			text_engine_composite()
				: text_engine(loader)
			{	}

			font_loader loader;
			gcontext::text_engine_type text_engine;
		};
		auto tec = make_shared<text_engine_composite>();

		return shared_ptr<gcontext::text_engine_type>(tec, &tec->text_engine);
	}
}
