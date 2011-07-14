#pragma once

namespace wpl
{
	namespace mt
	{
		struct waitable
		{
			enum wait_status { satisfied, timeout, abandoned };

			static const unsigned int infinite = static_cast<unsigned int>(-1);

			virtual wait_status wait(unsigned int timeout = infinite) volatile = 0;
		};
	}
}
