cmake_minimum_required(VERSION 2.8)

include(test)

set(WPL_TEST_SOURCES
	AnimatedModelsTests.cpp
	DragHelperTests.cpp
	FactoryTests.cpp
	HeaderCoreTests.cpp
	helpers.cpp
	helpers-visual.cpp
	KeyboardRouterTests.cpp
	LayoutTests.cpp
	ListViewCoreSelectionTests.cpp
	ListViewCoreTests.cpp
	Mockups.cpp
	MockupsListView.cpp
	MouseRouterTests.cpp
	ScrollerTests.cpp
	SignalTests.cpp
	StylesheetTests.cpp
	VisualRouterTests.cpp
	VisualTests.cpp
)

if (WIN32)
	set(WPL_TEST_SOURCES ${WPL_TEST_SOURCES}
		helpers-win32.cpp
		IntegrationTests.cpp
		KeyboardRouterNativeTests.cpp
		LayoutTests.cpp
		MockupsNative.cpp
		NativeComboboxTests.cpp
		NativeControlsTests.cpp
		NativeCursorManagementTests.cpp
		NativeFontManagerTests.cpp
		NativeFormTests.cpp
		NativeListViewTests.cpp
		NativeViewHostTests.cpp
		NativeViewTests.cpp
		WindowingTests.cpp
	)

	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /MANIFESTDEPENDENCY:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
endif()

add_library(wpl.tests SHARED ${WPL_TEST_SOURCES})

target_link_libraries(wpl.tests wpl agge.text agge)
