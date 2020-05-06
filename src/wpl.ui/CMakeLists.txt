cmake_minimum_required(VERSION 2.8)

set(WPL_UI_SOURCES
	container.cpp
	layout.cpp
	scroller.cpp
	visual.cpp
)

if(WIN32)
	set(WPL_UI_SOURCES ${WPL_UI_SOURCES}
		win32/comboxbox.cpp
		win32/controls.cpp
		win32/form.cpp
		win32/listview.cpp
		win32/view_host.cpp
		win32/window.cpp
	)
endif()

add_library(wpl.ui STATIC ${WPL_UI_SOURCES})

target_link_libraries(wpl.ui agge)
