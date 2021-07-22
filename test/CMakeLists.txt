cmake_minimum_required(VERSION 3.14)

project(gif_engineTests LANGUAGES C)

include(../cmake/project-is-top-level.cmake)

if(PROJECT_IS_TOP_LEVEL)
  find_package(gif_engine REQUIRED)
  enable_testing()
endif()

find_package(utest REQUIRED)
include(utest_discover_tests)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED YES)

include_directories(source)

add_library(gif_engine_mmap OBJECT)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_sources(gif_engine_mmap PRIVATE source/gif_mmap.nt.c)
  target_compile_definitions(gif_engine_mmap PRIVATE WIN32_LEAN_AND_MEAN)
  if(NOT CMAKE_VERSION VERSION_LESS "3.16")
    target_precompile_headers(gif_engine_mmap PRIVATE <Windows.h>)
  endif()
else()
  target_sources(gif_engine_mmap PRIVATE source/gif_mmap.posix.c)
endif()

add_executable(
    gif_engine_test
    source/gif_engine_test.c
)
target_link_libraries(
    gif_engine_test PRIVATE
    gif_engine_mmap gif_engine::gif_engine utest::utest
)

utest_discover_tests(
    gif_engine_test
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/assets"
    DEPENDS gif_engine::gif_engine
)
