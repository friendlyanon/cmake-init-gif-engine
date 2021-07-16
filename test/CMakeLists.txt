cmake_minimum_required(VERSION 3.14)

project(gif_engineTests LANGUAGES C)

include(../cmake/project-is-top-level.cmake)

if(PROJECT_IS_TOP_LEVEL)
  find_package(gif_engine REQUIRED)
  enable_testing()
endif()

find_package(utest REQUIRED)
include(utest_discover_tests)

add_executable(
    gif_engine_test
    source/gif_engine_test.c
)
target_link_libraries(
    gif_engine_test PRIVATE
    gif_engine::gif_engine utest::utest
)
target_compile_features(gif_engine_test PRIVATE c_std_11)

utest_discover_tests(gif_engine_test DEPENDS gif_engine::gif_engine)
