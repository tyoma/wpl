cmake_minimum_required(VERSION 3.13)

add_subdirectory(common/src)
link_libraries(samples.common)

add_executable(controls controls/main.cpp)
add_executable(layout layout/main.cpp)
add_executable(listview listview/main.cpp)
add_executable(overlay overlay/main.cpp)
add_executable(piechart piechart/main.cpp piechart/piechart.cpp)
add_executable(taborder taborder/main.cpp)

if (APPLE)
	set_target_properties(controls layout listview overlay piechart taborder PROPERTIES
		MACOSX_BUNDLE YES
		MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/common/src/info.plist
		XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.agge.sample"
		XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
		XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
		XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
	)
endif()
