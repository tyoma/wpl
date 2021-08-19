#pragma once

#include <set>
#include <wpl/models.h>

namespace wpl
{
	namespace tests
	{
		namespace mocks
		{
			class dynamic_set_model : public wpl::dynamic_set_model
			{
			public:
				std::set<index_type> items;

			private:
				virtual void clear() throw() override
				{	items.clear();	}

				virtual void add(index_type item) override
				{	items.insert(item);	}

				virtual void remove(index_type item) override
				{	items.erase(item);	}

				virtual bool contains(index_type item) const throw() override
				{	return !!items.count(item);	}
			};
		}
	}
}
