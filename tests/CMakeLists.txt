cmake_minimum_required(VERSION 2.8)

include(test)

set(WPL_TEST_SOURCES
	AnimatedModelsTests.cpp
	DragHelperTests.cpp
	FactoryTests.cpp
	HeaderCoreTests.cpp
	KeyboardRouterTests.cpp
	LayoutTests.cpp
	ListViewCoreSelectionTests.cpp
	ListViewCoreTests.cpp
	MouseRouterTests.cpp
	ScrollerTests.cpp
	SignalBaseTests.cpp
	SignalTests.cpp
	StylesheetTests.cpp
	VisualRouterTests.cpp
	VisualTests.cpp
)

add_subdirectory(common)
if(WIN32)
	add_subdirectory(win32)
endif()

add_library(wpl.tests SHARED ${WPL_TEST_SOURCES})

target_link_libraries(wpl.tests wpl-tests-common wpl agge.text agge)
