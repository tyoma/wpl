#include <crtdbg.h>

#include <wpl/ui/button.h>
#include <wpl/ui/form.h>
#include <wpl/ui/layout.h>
#include <wpl/ui/listview.h>

namespace std
{
	using tr1::static_pointer_cast;
}

using namespace std;
using namespace wpl;
using namespace wpl::ui;

void init_platform();
void run_message_loop();
void exit_message_loop();

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	init_platform();

	shared_ptr<form> f = form::create();
	shared_ptr<container> root = f->get_root_container();
	slot_connection c = f->close += &exit_message_loop;

	int vsizes[] = { 150, -100, 150, 25, };
	int button_sizes[] = { 100, 100, };

	root->layout.reset(new vstack(vsizes, vsizes + _countof(vsizes), 3));

	shared_ptr<listview> lv_parents = static_pointer_cast<listview>(root->create_widget(L"listview", L"parents"));
	shared_ptr<listview> lv_main = static_pointer_cast<listview>(root->create_widget(L"listview", L"main"));
	shared_ptr<listview> lv_children = static_pointer_cast<listview>(root->create_widget(L"listview", L"children"));
	shared_ptr<container> buttons = root->create_container(L"buttons");

	buttons->layout.reset(new hstack(button_sizes, button_sizes + _countof(button_sizes), 3));

	buttons->create_widget(L"button", L"1");
	buttons->create_widget(L"button", L"2");



	f->set_visible(true);
	run_message_loop();
}
