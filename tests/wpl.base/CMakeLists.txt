cmake_minimum_required(VERSION 2.8)

include(test)

set(WPL_BASE_TEST_SOURCES
	SignalTests.cpp
)

add_library(wpl.base.tests SHARED ${WPL_BASE_TEST_SOURCES})

target_link_libraries(wpl.base.tests utee)
