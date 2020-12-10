cmake_minimum_required(VERSION 2.8)

set(WPL_SOURCES
	animated_models.cpp
	animation.cpp
	factory.cpp
	glyphs.cpp
	input_stubs.cpp
	keyboard_router.cpp
	layout.cpp
	mouse_router.cpp
	stylesheet_db.cpp
	visual.cpp
	visual_router.cpp

	controls/background.cpp
	controls/header_basic.cpp
	controls/header_core.cpp
	controls/listview_basic.cpp
	controls/listview_core.cpp
	controls/scroller.cpp
)

if(WIN32)
	set(WPL_SOURCES ${WPL_SOURCES}
		win32/combobox.cpp
		win32/controls.cpp
		win32/cursor_manager.cpp
		win32/factory_win32.cpp
		win32/font_loader_win32.cpp
		win32/font_manager_win32.cpp
		win32/form.cpp
		win32/listview_win32.cpp
		win32/native_view.cpp
		win32/queue.cpp
		win32/view_host.cpp
		win32/window.cpp
	)
elseif(APPLE)
	set(WPL_SOURCES ${WPL_SOURCES}
		macos/factory_macos.cpp
	)
endif()

add_library(wpl STATIC ${WPL_SOURCES})

target_link_libraries(wpl agge.text agge)
