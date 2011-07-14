#pragma once

#include "concepts.h"

namespace wpl
{
	namespace mt
	{
		class event_flag : waitable
		{
			void *_handle;

		public:
			explicit event_flag(bool raised, bool auto_reset);
			~event_flag();

			void raise();
			void lower();

			virtual wait_status wait(unsigned int timeout = infinite) volatile;
		};
	}
}
