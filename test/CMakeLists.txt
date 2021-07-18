cmake_minimum_required(VERSION 3.14)

project(gif_engineTests LANGUAGES C)

include(../cmake/project-is-top-level.cmake)

if(PROJECT_IS_TOP_LEVEL)
  find_package(gif_engine REQUIRED)
  enable_testing()
endif()

find_package(utest REQUIRED)
include(utest_discover_tests)

link_libraries(gif_engine::gif_engine utest::utest)
include_directories(source)

function(def_test name)
  add_executable("${name}" ${ARGN})
  target_compile_features("${name}" PRIVATE c_std_11)
endfunction()

set(mmap_variant posix)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(mmap_variant nt)
endif()

def_test(
    gif_engine_test
    source/gif_engine_test.c
    "source/gif_mmap.${mmap_variant}.c"
)

utest_discover_tests(gif_engine_test DEPENDS gif_engine::gif_engine)
