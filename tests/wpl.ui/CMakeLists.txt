cmake_minimum_required(VERSION 2.8)

include(test)

set(WPL_UI_TEST_SOURCES
	ContainerTests.cpp
	FactoryTests.cpp
	HeaderTests.cpp
	helpers.cpp
	helpers-visual.cpp
	LayoutTests.cpp
	ListViewCoreTests.cpp
	Mockups.cpp
	MockupsListView.cpp
	ScrollerTests.cpp
	VisualTests.cpp
)

if (WIN32)
	set(WPL_UI_TEST_SOURCES ${WPL_UI_TEST_SOURCES}
		FormTests.cpp
		helpers-win32.cpp
		IntegrationTests.cpp
		LayoutTests.cpp
		MockupsNative.cpp
		NativeComboboxTests.cpp
		NativeControlsTests.cpp
		NativeListViewTests.cpp
		NativeViewHostTests.cpp
		NativeViewTests.cpp
		ScrollerTests.cpp
		WindowingTests.cpp
	)

	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /MANIFESTDEPENDENCY:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
endif()

add_library(wpl.ui.tests SHARED ${WPL_UI_TEST_SOURCES})

target_link_libraries(wpl.ui.tests wpl.ui agge)
