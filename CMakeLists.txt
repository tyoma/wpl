cmake_minimum_required(VERSION 2.8)

project(wpl)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/modules)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_bin)
set(AGGE_NO_TESTS 1)
set(UTEE_NO_TESTS 1)

if (MSVC)
	add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS)
	add_definitions(-DUNICODE -D_UNICODE -DISOLATION_AWARE_ENABLED=1)
endif()

add_compile_options(
	$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:$<$<COMPILE_LANGUAGE:C>:-fPIC>>
	$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:$<$<COMPILE_LANGUAGE:CXX>:-fPIC>>
)

if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.13.5)
	add_link_options(
		"$<$<CXX_COMPILER_ID:GNU>:-Wl,--no-undefined>"
		"$<$<CXX_COMPILER_ID:Clang>:-Wl,-undefined,error>"
	)
endif()

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

include_directories(.)
include_directories(./libraries/agge)

add_subdirectory(src)
if (NOT TARGET agge)
	add_subdirectory(libraries/agge)
endif()
if (NOT WPL_NO_TESTS)
	if (NOT TARGET utee)
		add_subdirectory(libraries/utee)
	endif()
	add_subdirectory(tests)
endif()
