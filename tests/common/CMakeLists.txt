cmake_minimum_required(VERSION 3.13)

set(WPL_TESTS_COMMON_SOURCES
	helpers.cpp
	helpers-visual.cpp
	Mockups.cpp
	MockupsListView.cpp
)

add_library(wpl-tests-common STATIC ${WPL_TESTS_COMMON_SOURCES})
target_link_libraries(wpl-tests-common agge utfia utee)
