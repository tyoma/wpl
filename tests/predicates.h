#pragma once

#include <agge/types.h>
#include <utility>

namespace wpl
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

			template <typename T1, typename T2>
			bool operator ()(const agge::rect<T1> &lhs, const agge::rect<T2> &rhs) const
			{
				return (*this)(lhs.x1, rhs.x1) && (*this)(lhs.y1, rhs.y1)
					&& (*this)(lhs.x2, rhs.x2) && (*this)(lhs.y2, rhs.y2);
			}

		private:
			double _tolerance;
		};
	}
}
