cmake_minimum_required(VERSION 2.8)

enable_language(OBJCXX)

set(CMAKE_OBJCXX_STANDARD 11)
	
set(WPL_MACOS_SOURCES
	factory_macos.cpp
	font_loader_freetype_unix.cpp
	form.mm
)

add_library(wpl.macos STATIC ${WPL_MACOS_SOURCES})

target_link_libraries(wpl.macos "-framework Foundation -framework Cocoa")
