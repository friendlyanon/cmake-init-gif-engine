cmake_minimum_required(VERSION 3.14)

include(cmake/in-source-guard.cmake)

project(
    gif_engine
    VERSION 0.0.0
    DESCRIPTION "GIF decoder in C"
    HOMEPAGE_URL "https://github.com/friendlyanon/cmake-init-gif-engine"
    LANGUAGES C
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
    gif_engine_gif_engine
    source/gif_engine.c
)
add_library(gif_engine::gif_engine ALIAS gif_engine_gif_engine)

include(GenerateExportHeader)
generate_export_header(
    gif_engine_gif_engine
    BASE_NAME gif_engine
    EXPORT_FILE_NAME export/gif_engine/export.h
)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(gif_engine_gif_engine PUBLIC GIF_ENGINE_STATIC_DEFINE)
endif()

set_target_properties(
    gif_engine_gif_engine PROPERTIES
    C_VISIBILITY_PRESET hidden
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME gif_engine
    OUTPUT_NAME gif_engine
)

target_include_directories(
    gif_engine_gif_engine ${gif_engine_warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    gif_engine_gif_engine SYSTEM
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(gif_engine_gif_engine PUBLIC c_std_11)

# ---- Install rules ----

include(cmake/install-rules.cmake)

# ---- Developer mode ----

if(NOT gif_engine_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of gif_engine"
  )
endif()

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()

option(BUILD_MCSS_DOCS "Build documentation using Doxygen and m.css" OFF)
if(BUILD_MCSS_DOCS)
  include(cmake/docs.cmake)
endif()

option(ENABLE_COVERAGE "Enable coverage support separate from CTest's" OFF)
if(ENABLE_COVERAGE)
  include(cmake/coverage.cmake)
endif()

include(cmake/lint-targets.cmake)
include(cmake/spell-targets.cmake)
