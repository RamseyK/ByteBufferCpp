cmake_minimum_required (VERSION 2.8 FATAL_ERROR)
set (CMAKE_LEGACY_CYGWIN32 0)

project (ByteBufferCpp)

set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)
set (VERSION_PATCH "0-sgtcodfish")

set (VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

set (ByteBufferCpp_SOURCES ${PROJECT_SOURCE_DIR}/src/ByteBuffer.cpp)
set (ByteBufferCpp_HEADERS ${PROJECT_SOURCE_DIR}/src/ByteBuffer.hpp)

set (ByteBufferCpp_TEST_SOURCES ${PROJECT_SOURCE_DIR}/src/test.cpp)

option (EXCLUDE_TEST "Ignore the test file" ON)

set (CMAKE_CXX_COMPILER_ARG1 "-std=c++1y")
set (CMAKE_CXX_FLAGS_BASE "-Wall -Wextra")

set (BB_DEBUG_FLAGS "-g3 -O0")
set (BB_RELEASE_FLAGS "-g0 -O3")

include_directories ("src")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_BASE} ${BB_DEBUG_FLAGS}")
add_library (ByteBufferCpp-d ${ByteBufferCpp_SOURCES})

if (NOT EXCLUDE_TEST)
	add_executable (ByteBufferCppTest ${ByteBufferCpp_TEST_SOURCES})
	target_link_libraries (ByteBufferCppTest ByteBufferCpp-d)
endif ()

if (NOT EXCLUDE_RELEASE)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_BASE} ${BB_RELEASE_FLAGS}")
	add_library (ByteBufferCpp ${ByteBufferCpp_SOURCES})
endif ()
