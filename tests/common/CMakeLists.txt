cmake_minimum_required(VERSION 2.8)

include(test)

set(WPL_TESTS_COMMON_SOURCES
	helpers.cpp
	helpers-visual.cpp
	Mockups.cpp
	MockupsListView.cpp
)

add_library(wpl-tests-common STATIC ${WPL_TESTS_COMMON_SOURCES})
