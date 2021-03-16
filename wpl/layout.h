//	Copyright (c) 2011-2021 by Artem A. Gevorkyan (gevorkyan.org)
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.

#pragma once

#include "concepts.h"
#include "control.h"
#include "types.h"

#include <vector>

namespace wpl
{
	struct cursor_manager;

	class container : public control, noncopyable
	{
	protected:
		void add(control &child);

	private:
		std::vector<wpl::slot_connection> _connections;
	};


	class stack : public container
	{
	public:
		stack(bool horizontal, std::shared_ptr<cursor_manager> cursor_manager_);

		void set_spacing(int spacing);
		void add(std::shared_ptr<control> child, display_unit size, bool resizable = false, int tab_order = 0);

		// control methods
		virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) override;

	private:
		struct item
		{
			std::shared_ptr<control> child;
			display_unit size;
			bool resizable;
			int tab_order;
		};

		class splitter;

	private:
		double get_rsize() const;
		void move_splitter(size_t index, double delta);

	private:
		std::vector<item> _children;
		std::vector< std::shared_ptr<splitter> > _splitters;
		const std::shared_ptr<cursor_manager> _cursor_manager;
		int _spacing;
		int _last_size;
		bool _horizontal;
	};


	class padding : public control
	{
	public:
		padding(std::shared_ptr<control> inner, int px, int py);

		virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) override;
		virtual int min_height(int for_width) const override;
		virtual int min_width(int for_height) const override;

	private:
		std::shared_ptr<control> _inner;
		slot_connection _connection;
		int _px, _py;
	};


	class overlay : public container
	{
	public:
		void add(std::shared_ptr<control> child);

		// control methods
		virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) override;
		virtual int min_height(int for_width) const override;
		virtual int min_width(int for_height) const override;

	private:
		std::vector< std::shared_ptr<control> > _children;
	};


	class staggered : public container
	{
	public:
		staggered();

		void add(std::shared_ptr<control> child);

		void set_base_width(display_unit width);

		// control methods
		virtual void layout(const placed_view_appender &append_view, const agge::box<int> &box) override;

	private:
		struct next
		{
			int bottom, x0, width;

			bool operator <(const next &rhs) const;
		};

	private:
		std::vector< std::shared_ptr<control> > _children;
		mutable std::vector<next> _next_items_buffer;
		display_unit _base_width;
	};



	std::shared_ptr<control> pad_control(std::shared_ptr<control> inner, int px, int py);
}
