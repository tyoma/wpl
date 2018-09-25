#pragma once

#include <cmath>

namespace wpl
{
	class animation_line
	{
	public:
		animation_line(float initial_value = 0.0f);

		void run(float destination_value, unsigned duration);
		float get_value() const;
		bool update(unsigned time);

	private:
		float _value, _start_value, _destination_value;
		unsigned _duration;
		unsigned _start_time;
		bool _done;
	};

	inline animation_line::animation_line(float initial_value)
		: _value(initial_value), _done(true)
	{	}

	inline void animation_line::run(float destination_value, unsigned duration)
	{
		_start_time = 0;
		_duration = duration;
		_start_value = _value;
		_destination_value = destination_value;
		_done = false;
	}

	inline float animation_line::get_value() const
	{	return _value;	}

	inline bool animation_line::update(unsigned time)
	{
		if (_done)
			return false;
		if (!_start_time)
			_start_time = time;
		float timed = (time - _start_time) / float(_duration);

		if (timed >= 1.0f)
		{
			timed = 1.0f;
			_done = true;
		}
		float k = std::sinf(3.1415926f * 0.5f * timed);
		_value = (1.0f - k) * _start_value + k * _destination_value;
		return !_done;
	}
}
