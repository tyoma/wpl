cmake_minimum_required(VERSION 3.13)

include(freetype2.include)

set(WPL_SOURCES
	animated_models.cpp
	animation.cpp
	drag_helper.cpp
	factory.cpp
	glyphs.cpp
	helpers.cpp
	input_stubs.cpp
	keyboard_router.cpp
	layout.cpp
	layout_stack.cpp
	layout_staggered.cpp
	mouse_router.cpp
	stylesheet_db.cpp
	visual.cpp
	visual_router.cpp

	controls/background.cpp
	controls/header_basic.cpp
	controls/header_core.cpp
	controls/label.cpp
	controls/listview_basic.cpp
	controls/listview_core.cpp
	controls/range_slider.cpp
	controls/scroller.cpp
	misc/statistics_view.cpp

	freetype2/font_loader.cpp
)

add_library(wpl.generic STATIC ${WPL_SOURCES})
target_link_libraries(wpl.generic PUBLIC agge.text agge)
target_link_libraries(wpl.generic PRIVATE CONAN_PKG::freetype)

add_library(wpl INTERFACE)
target_include_directories(wpl INTERFACE ..)
target_link_libraries(wpl INTERFACE wpl.generic)
if(WIN32)
	add_subdirectory(win32)
	target_link_libraries(wpl INTERFACE wpl.win32)
elseif(APPLE)
	add_subdirectory(macos)
	target_link_libraries(wpl INTERFACE wpl.macos)
endif()
