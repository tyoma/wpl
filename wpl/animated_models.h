//	Copyright (c) 2011-2020 by Artem A. Gevorkyan (gevorkyan.org)
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

#include "animation.h"
#include "concepts.h"
#include "models.h"
#include "queue.h"

namespace wpl
{
	class animated_scroll_model : public scroll_model, noncopyable
	{
	public:
		animated_scroll_model(std::shared_ptr<scroll_model> underlying, const clock &clock_, const queue &queue_,
			const animation_function &release_animation);

		virtual std::pair<double, double> get_range() const override;
		virtual std::pair<double, double> get_window() const override;
		virtual double get_increment() const override;
		virtual void scrolling(bool begins) override;
		virtual void scroll_window(double window_min, double window_width) override;

	private:
		void animate();
		void on_invalidate(bool invalidate_range);

	private:
		timestamp _animation_start;
		double _excess;

		const clock _clock;
		const queue _queue;
		const animation_function _release_animation;
		const std::shared_ptr<scroll_model> _underlying;
		const slot_connection _invalidate_connection;
	};
}
