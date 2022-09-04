cmake_minimum_required(VERSION 2.8)

set(WPL_WIN32_SOURCES
	combobox.cpp
	controls.cpp
	cursor_manager.cpp
	factory_win32.cpp
	font_loader_freetype_win32.cpp
	font_loader_win32.cpp
	font_manager_win32.cpp
	form.cpp
	helpers_win32.cpp
	listview_win32.cpp
	mouse_router_win32.cpp
	native_view.cpp
	utf8.cpp
	view_host.cpp
	visual_router_win32.cpp
	window.cpp
)

add_library(wpl.win32 STATIC ${WPL_WIN32_SOURCES})
target_link_libraries(wpl.win32 agge utfia)
