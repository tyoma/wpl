cmake_minimum_required(VERSION 3.13)

set(WPL_TEST_SOURCES
	AnimatedModelsTests.cpp
	DragHelperTests.cpp
	FactoryTests.cpp
	GroupHeadersModelTests.cpp
	HeaderCoreTests.cpp
	KeyboardRouterTests.cpp
	LayoutTests.cpp
	ListViewCoreSelectionTests.cpp
	ListViewCoreTests.cpp
	MiscTests.cpp
	MouseRouterTests.cpp
	RangeSliderTests.cpp
	ScrollerTests.cpp
	SignalBaseTests.cpp
	SignalTests.cpp
	StackLayoutTests.cpp
	StaggeredLayoutTests.cpp
	StylesheetTests.cpp
	VisualRouterTests.cpp
	VisualTests.cpp
)

add_subdirectory(common)
if(WIN32)
	add_subdirectory(win32)
endif()

add_library(wpl.generic.tests SHARED ${WPL_TEST_SOURCES})

target_link_libraries(wpl.generic.tests wpl-tests-common wpl.generic)
