#pragma once

#include <utility>

namespace wpl
{
	namespace ui
	{
		namespace tests
		{
			class eq
			{
			public:
				eq(double tolerance = 0.0001)
					: _tolerance(tolerance)
				{	}

				bool operator ()(double lhs, double rhs) const
				{
					if (double e = 2 * (lhs - rhs))
					{
						e /= lhs + rhs;
						return -_tolerance <= e && e <= _tolerance;
					}
					return true;
				}

				bool operator ()(std::pair<double, double> lhs, std::pair<double, double> rhs) const
				{	return (*this)(lhs.first, rhs.first) && (*this)(lhs.second, rhs.second);	}

			private:
				double _tolerance;
			};
		}
	}
}
