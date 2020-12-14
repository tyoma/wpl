cmake_minimum_required(VERSION 2.8)

set(SAMPLES_COMMON_SOURCES
	stylesheet.cpp
)

if (WIN32)
	set(SAMPLES_COMMON_SOURCES ${SAMPLES_COMMON_SOURCES}
		application_win32.cpp
		text_engine_win32.cpp
		timer_win32.cpp
	)
elseif(APPLE)
	enable_language(OBJCXX)
	
	set(SAMPLES_COMMON_SOURCES ${SAMPLES_COMMON_SOURCES}
		application_macos.mm
		text_engine_macos.cpp
		timer_macos.cpp
	)
endif()

add_library(samples.common STATIC ${SAMPLES_COMMON_SOURCES})

target_link_libraries(samples.common wpl)
