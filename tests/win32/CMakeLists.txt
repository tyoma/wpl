cmake_minimum_required(VERSION 2.8)

include(test)

set(WPL_TEST_SOURCES
	ComboboxTests.cpp
	ControlsTests.cpp
	CursorManagementTests.cpp
	FontManagerTests.cpp
	FormTests.cpp
	helpers-win32.cpp
	IntegrationTests.cpp
	KeyboardRouterTests.cpp
	ListViewTests.cpp
	MockupsNative.cpp
	NativeViewTests.cpp
	ViewHostOverlayTests.cpp
	ViewHostTests.cpp
	WindowingTests.cpp
)

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /MANIFESTDEPENDENCY:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

add_library(wpl.win32.tests SHARED ${WPL_TEST_SOURCES})

target_link_libraries(wpl.win32.tests wpl-tests-common wpl agge.text agge)
