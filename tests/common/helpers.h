#pragma once

#include <memory>
#include <vector>
#include <ut/assert.h>
#include <wpl/control.h>

namespace wpl
{
	struct control;

	namespace tests
	{
		struct plural_
		{
			template <typename T>
			std::vector<T> operator +(const T &rhs) const
			{	return std::vector<T>(1, rhs);	}
		} const plural;



		template <typename T>
		inline std::vector<T> operator +(std::vector<T> lhs, const T &rhs)
		{	return lhs.push_back(rhs), lhs;	}

		template <typename T>
		std::function<void (const T &element)> make_appender(std::vector<T> &v)
		{
			return [&v] (const T &element) {
				v.push_back(element);
			};
		}

		template <typename ControlT>
		inline void resize(ControlT &control_, int cx, int cy)
		{
			std::shared_ptr<control> c(&control_, [] (void *) {});
			std::vector<placed_view> v;
			agge::box<int> b = { cx, cy };

			c->layout(make_appender(v), b);
			assert_equal(1u, v.size());
			assert_not_null(v[0].regular);
			assert_is_false(v[0].overlay);
			assert_equal(0, v[0].location.x1);
			assert_equal(0, v[0].location.y1);
			assert_equal(cx, v[0].location.x2);
			assert_equal(cy, v[0].location.y2);
		}

		template <typename ControlT>
		inline bool provides_tabstoppable_view(ControlT &control_)
		{
			std::shared_ptr<control> c(&control_, [] (void *) {});
			std::vector<placed_view> v;
			agge::box<int> b = { 1, 1 };

			c->layout(make_appender(v), b);
			assert_equal(1u, v.size());
			assert_is_true(v[0].tab_order == 1 || v[0].tab_order == 0);
			return !!v[0].tab_order;
		}

		template <typename T, size_t n>
		inline std::vector<T> mkvector(T (&data)[n])
		{	return std::vector<T>(data, data + n);	}
	}
}
