cmake_minimum_required(VERSION 3.14)

project(gif_engineTests LANGUAGES C)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)
include(../cmake/functions.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(gif_engine REQUIRED)
  enable_testing()
endif()

find_package(utest REQUIRED)
include(utest_discover_tests)

# ---- Quarantine OS specific functionality ----

include_directories(source)

add_library(gif_engine_mmap OBJECT)
target_sources_grouped(
    gif_engine_mmap TREE "${PROJECT_SOURCE_DIR}" FILES
    source/gif_mmap.h
)

if(WIN32)
  target_sources_grouped(
      gif_engine_mmap TREE "${PROJECT_SOURCE_DIR}" FILES
      source/gif_mmap.nt.c
  )
  target_compile_definitions(gif_engine_mmap PRIVATE WIN32_LEAN_AND_MEAN)
  if(NOT CMAKE_VERSION VERSION_LESS "3.16")
    target_precompile_headers(gif_engine_mmap PRIVATE <Windows.h>)
    source_group(Precompiled REGULAR_EXPRESSION "cmake_pch\\.[ch]")
  endif()
else()
  target_sources_grouped(
      gif_engine_mmap TREE "${PROJECT_SOURCE_DIR}" FILES
      source/gif_mmap.posix.c
  )
endif()

# ---- Test target ----

add_executable(gif_engine_test)
target_sources_grouped(
    gif_engine_test TREE "${PROJECT_SOURCE_DIR}" FILES
    source/gif_engine_test.c
)

target_link_libraries(
    gif_engine_test PRIVATE
    gif_engine_mmap gif_engine::gif_engine utest::utest
)

# ---- End-of-file commands ----

utest_discover_tests(
    gif_engine_test
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/assets"
    DEPENDS gif_engine::gif_engine
)

add_folders(Test)
