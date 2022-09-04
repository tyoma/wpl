cmake_minimum_required(VERSION 3.13)

project(wpl)

enable_testing()
cmake_policy(SET CMP0079 NEW)

option(WPL_NO_TESTS "Do not build test modules." OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_bin)

include_directories(BEFORE .)

if (MSVC)
	add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS)
	add_definitions(-DUNICODE -D_UNICODE -DISOLATION_AWARE_ENABLED=1)
endif()

add_compile_options(
	"$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fPIC>"
	"$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:$<$<COMPILE_LANGUAGE:CXX>:-std=c++11;-fno-rtti>>"
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

if (NOT TARGET freetype)
	add_subdirectory(libraries/freetype2)
endif()
if (NOT TARGET utfia)
	set(UTFIA_NO_TESTS ON)
	add_subdirectory(libraries/utfia)
endif()
if (NOT TARGET agge)
	set(AGGE_NO_TESTS ON)
	set(AGGE_NO_EXTENSIONS ON)
	add_subdirectory(libraries/agge)
endif()

add_subdirectory(src)
if (NOT WPL_NO_TESTS)
	if (NOT TARGET utee)
		set(UTEE_NO_TESTS ON)
		add_subdirectory(libraries/utee)
	endif()

	add_subdirectory(tests)

	add_utee_test(wpl.tests)
	if (WIN32)
		add_utee_test(wpl.win32.tests)
	endif()
	if (WIN32 OR APPLE)
		add_subdirectory(samples)
	endif()
endif()
