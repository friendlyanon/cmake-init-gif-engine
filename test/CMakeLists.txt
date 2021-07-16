cmake_minimum_required(VERSION 3.14)

project(gif_engineTests LANGUAGES C)

include(../cmake/project-is-top-level.cmake)
include(../cmake/windows-set-path.cmake)

if(PROJECT_IS_TOP_LEVEL)
  find_package(gif_engine REQUIRED)
  enable_testing()
endif()

add_executable(gif_engine_test source/gif_engine_test.c)
target_link_libraries(gif_engine_test PRIVATE gif_engine::gif_engine)
target_compile_features(gif_engine_test PRIVATE c_std_11)

add_test(NAME gif_engine_test COMMAND gif_engine_test)
windows_set_path(gif_engine_test gif_engine::gif_engine)
