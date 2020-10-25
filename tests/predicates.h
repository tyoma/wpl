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

			bool operator ()(float lhs, float rhs) const
			{	return (*this)(static_cast<double>(lhs), static_cast<double>(rhs));	}

			bool operator ()(double lhs, double rhs) const
			{
				if (double e = 2 * (lhs - rhs))
				{
					e /= lhs + rhs;
					return -_tolerance <= e && e <= _tolerance;
				}
				return true;
			}

			template <typename T1, typename T2, typename T3, typename T4>
			bool operator ()(std::pair<T1, T2> lhs, std::pair<T3, T4> rhs) const
			{
				return (*this)(static_cast<double>(lhs.first), static_cast<double>(rhs.first))
					&& (*this)(static_cast<double>(lhs.second), static_cast<double>(rhs.second));
			}

			template <typename T1, typename T2>
			bool operator ()(const agge::rect<T1> &lhs, const agge::rect<T2> &rhs) const
			{
				return (*this)(static_cast<double>(lhs.x1), static_cast<double>(rhs.x1))
					&& (*this)(static_cast<double>(lhs.y1), static_cast<double>(rhs.y1))
					&& (*this)(static_cast<double>(lhs.x2), static_cast<double>(rhs.x2))
					&& (*this)(static_cast<double>(lhs.y2), static_cast<double>(rhs.y2));
			}

		private:
			double _tolerance;
		};
	}
}
