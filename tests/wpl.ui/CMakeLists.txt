cmake_minimum_required(VERSION 2.8)

include(test)

set(WPL_UI_TEST_SOURCES
	ComboboxTests.cpp
	ContainerTests.cpp
	FormTests.cpp
	IntegrationTests.cpp
	LayoutTests.cpp
	ListViewTests.cpp
	Mockups.cpp
	MockupsListView.cpp
	MockupsNative.cpp
	NativeControlsTests.cpp
	NativeViewHostTests.cpp
	NativeViewTests.cpp
	TestHelpers.cpp
	WindowingTests.cpp
)

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /MANIFESTDEPENDENCY:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

add_library(wpl.ui.tests SHARED ${WPL_UI_TEST_SOURCES})

target_link_libraries(wpl.ui.tests wpl.ui agge)
