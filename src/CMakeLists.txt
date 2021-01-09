cmake_minimum_required(VERSION 2.8)

set(WPL_SOURCES
	animated_models.cpp
	animation.cpp
	factory.cpp
	glyphs.cpp
	helpers.cpp
	input_stubs.cpp
	keyboard_router.cpp
	layout.cpp
	mouse_router.cpp
	resizable_stack.cpp
	stylesheet_db.cpp
	visual.cpp
	visual_router.cpp

	controls/background.cpp
	controls/header_basic.cpp
	controls/header_core.cpp
	controls/listview_basic.cpp
	controls/listview_core.cpp
	controls/scroller.cpp

	freetype2/font_loader.cpp
)

add_library(wpl STATIC ${WPL_SOURCES})

if(WIN32)
	add_subdirectory(win32)
	target_link_libraries(wpl wpl.win32)
elseif(APPLE)
	add_subdirectory(macos)
	target_link_libraries(wpl wpl.macos)
endif()

target_link_libraries(wpl agge.text agge freetype)
