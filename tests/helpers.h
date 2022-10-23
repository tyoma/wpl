#pragma once

#include <ut/assert.h>
#include <wpl/input.h>

namespace wpl
{
	namespace tests
	{
		class capture_source
		{
		public:
			capture_source()
				: _target(nullptr)
			{	}

			void attach_to(mouse_input &originator)
			{
				_connection = originator.capture += [this] (std::shared_ptr<void> &handle, mouse_input &target_) {
					auto self = this;

					assert_null(_target);
					_target = &target_;
					handle.reset(new int, [self] (int *p) {
						self->_target = nullptr;
						delete p;
					});
				};
			}

		public:
			mouse_input *target()
			{	return _target;	}

		private:
			mouse_input *_target;
			wpl::slot_connection _connection;

		};
	}
}
