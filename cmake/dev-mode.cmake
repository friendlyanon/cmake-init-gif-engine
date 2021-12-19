include(cmake/folders.cmake)

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

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  include(cmake/open-cpp-coverage.cmake OPTIONAL)
endif()

include(cmake/lint-targets.cmake)
include(cmake/spell-targets.cmake)

# This target can be used to bring generated sources up-to-date without running
# the entire build process for generators that don't know about file targets
add_custom_target(
    generate-sources DEPENDS
    "${PROJECT_BINARY_DIR}/result_code.c"
    COMMENT "Generating sources added with add_custom_command"
)

# Add headers to targets in dev mode only, so IDEs that do not use the
# filesystem view for displaying targets have the headers associated as well
target_sources_grouped(
    gif_engine_gif_engine TREE "${PROJECT_SOURCE_DIR}" FILES
    include/gif_engine/error.h
    include/gif_engine/gif_engine.h
    include/gif_engine/structs.h
    source/binary_literal.h
    source/buffer_ops.h
    source/try.h
    source/decode/decode.h
    source/parse/parse.h
    source/parse/parse_state.h
)

source_group(
    TREE "${PROJECT_SOURCE_DIR}" FILES
    include/gif_engine/result_code.h
)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT gif_engine_gif_engine)

add_folders(Project)
