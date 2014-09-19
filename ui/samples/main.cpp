#include <wpl/ui/button.h>
#include <wpl/ui/form.h>
#include <wpl/ui/layout.h>
#include <wpl/ui/listview.h>

using namespace std;
using namespace wpl::ui;

void run_message_loop();
void exit_message_loop();

namespace
{
	template <int (layout_manager::position::*Position), int (layout_manager::position::*Size)>
	void resize_widgets_stacked(int requested_size, int common_size, const int *sizes, size_t sizes_count,
		layout_manager::widget_position *positions, size_t count);

	class vstack : public layout_manager
	{
	private:
		virtual ~vstack() throw()	{	}

		virtual void layout(size_t width, size_t height, widget_position *widgets, size_t count) const
		{
			resize_widgets_stacked<&layout_manager::position::top, &layout_manager::position::height>(height, width,
				&_sizes[0], _sizes.size(), widgets, count);
		}

	private:
		vector<int> _sizes;
	};
}

int main()
{
	shared_ptr<form> f = form::create();

	f->set_visible(true);
	run_message_loop();
}
