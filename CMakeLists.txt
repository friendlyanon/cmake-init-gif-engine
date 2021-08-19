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

# ---- Generate gif_result_code stringifyer function ----

add_custom_command(
    OUTPUT "${PROJECT_BINARY_DIR}/result_code.c"
    COMMAND "${CMAKE_COMMAND}"
    -D "input=${PROJECT_SOURCE_DIR}/include/gif_engine/result_code.h"
    -D "output=${PROJECT_BINARY_DIR}/result_code.c"
    -P "${PROJECT_SOURCE_DIR}/cmake/stringify-result-code.cmake"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${PROJECT_BINARY_DIR}/result_code.c.tmp"
    "${PROJECT_BINARY_DIR}/result_code.c"
    COMMAND "${CMAKE_COMMAND}" -E remove
    "${PROJECT_BINARY_DIR}/result_code.c.tmp"
    MAIN_DEPENDENCY include/gif_engine/result_code.h
    DEPENDS "${PROJECT_SOURCE_DIR}/cmake/stringify-result-code.cmake"
    COMMENT "Generating stringifyer function for gif_result_code"
    VERBATIM
)

# ---- Declare library ----

add_library(
    gif_engine_gif_engine
    source/gif_engine.c
    source/buffer_ops.c
    source/decode.c
    source/parse/parse.c
    source/last_position.c
    "${PROJECT_BINARY_DIR}/result_code.c"
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

target_include_directories(gif_engine_gif_engine PRIVATE source)

target_include_directories(
    gif_engine_gif_engine SYSTEM
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(gif_engine_gif_engine PUBLIC c_std_11)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT gif_engine_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of gif_engine"
  )
endif()

include(cmake/dev-mode.cmake)
