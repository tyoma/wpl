#pragma once

#include <agge.text/richtext.h>

namespace wpl
{
	struct modify_family
	{
		modify_family(const std::string &family_)
			: family(family_)
		{	}

		void operator ()(agge::richtext_t &target) const
		{
			auto a = target.current_annotation();

			if (a.basic.family != family)
			{
				a.basic.family = family;
				target.annotate(a);
			}
		}

		std::string family;
	};

	struct modify_height
	{
		modify_height(int height_)
			: height(height_)
		{	}

		void operator ()(agge::richtext_t &target) const
		{
			auto a = target.current_annotation();

			if (a.basic.height != height)
			{
				a.basic.height = height;
				target.annotate(a);
			}
		}

		int height;
	};

	struct modify_weight
	{
		modify_weight(bool bold_)
			: bold(bold_)
		{	}

		void operator ()(agge::richtext_t &target) const
		{
			auto a = target.current_annotation();

			if (!!a.basic.bold != bold)
			{
				a.basic.bold = bold;
				target.annotate(a);
			}
		}

		bool bold;
	};



	template <typename StyleModifier>
	inline agge::richtext_t &operator <<(agge::richtext_t &lhs, const StyleModifier &rhs)
	{	return rhs(lhs), lhs;	}

	inline agge::richtext_t &operator <<(agge::richtext_t &lhs, const wchar_t *rhs)
	{	return lhs += rhs, lhs;	}

	inline agge::richtext_t &operator <<(agge::richtext_t &lhs, const std::wstring &rhs)
	{	return lhs += rhs, lhs;	}
}
