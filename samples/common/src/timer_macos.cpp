#include "../timer.h"

using namespace std;

namespace wpl
{
	shared_ptr<void> create_timer(unsigned /*timeout*/, const function<void(unsigned elapsed)> &/*callback*/)
	{	return nullptr;	}
}
