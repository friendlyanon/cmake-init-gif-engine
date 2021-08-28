cmake_minimum_required(VERSION 3.14)

project(gif_engineFuzzSuperbuild LANGUAGES NONE)

if(NOT CMAKE_VERSION VERSION_LESS "3.19")
  cmake_policy(SET CMP0114 NEW)
  macro(ensure_dependencies)
  endmacro()
else()
  macro(ensure_dependencies p)
    foreach(step IN ITEMS configure build install)
      if(NOT TARGET "${p}-${step}")
        ExternalProject_Add_StepTargets("${p}" "${step}")
      endif()
      if(step STREQUAL "install")
        ExternalProject_Add_StepDependencies("${p}" install "${p}-build")
      elseif(step STREQUAL "build")
        ExternalProject_Add_StepDependencies("${p}" build "${p}-configure")
      endif()
    endforeach()
  endmacro()
endif()

# Allow the Ninja generators to output messages as they happen by assigning
# these jobs to the 'console' job pool
set(console_access "")
if(CMAKE_GENERATOR MATCHES "^Ninja")
  set(
      console_access
      USES_TERMINAL_CONFIGURE YES
      USES_TERMINAL_BUILD YES
      USES_TERMINAL_INSTALL YES
  )
endif()

include(ExternalProject)

set_property(DIRECTORY PROPERTY EP_PREFIX "${PROJECT_BINARY_DIR}/ep-prefix")

set(flags "-O2 -g -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer \
-fno-common")
set(install_dir "${PROJECT_BINARY_DIR}/gif-engine-install")

ExternalProject_Add(
    gif-engine
    SOURCE_DIR "${PROJECT_SOURCE_DIR}/.."
    BINARY_DIR "${PROJECT_BINARY_DIR}/gif-engine-build"
    INSTALL_DIR "${install_dir}"
    CMAKE_CACHE_ARGS
    -DBUILD_SHARED_LIBS:STRING=NO
    "-DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}"
    CMAKE_CACHE_DEFAULT_ARGS
    -DCMAKE_BUILD_TYPE:STRING=Fuzz
    "-DCMAKE_C_FLAGS_FUZZ:STRING=${flags}"
    -DCMAKE_INSTALL_PREFIX:STRING=<INSTALL_DIR>
    STEP_TARGETS install
    BUILD_ALWAYS YES
    EXCLUDE_FROM_ALL YES
    ${console_access}
)

ensure_dependencies(gif-engine)

foreach(fuzzer IN ITEMS parser)
  set(src "${PROJECT_BINARY_DIR}/${fuzzer}-src")
  configure_file("${fuzzer}.c" "${src}/main.c" COPYONLY)
  configure_file(CMakeLists.txt.in "${src}/CMakeLists.txt" @ONLY)
  ExternalProject_Add(
      "${fuzzer}"
      SOURCE_DIR "${src}"
      BINARY_DIR "${PROJECT_BINARY_DIR}/${fuzzer}-build"
      CMAKE_CACHE_ARGS
      "-DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}"
      CMAKE_CACHE_DEFAULT_ARGS
      -DCMAKE_BUILD_TYPE:STRING=Fuzz
      -DCMAKE_MAP_IMPORTED_CONFIG_FUZZ:STRING=Fuzz
      "-DCMAKE_C_FLAGS_FUZZ:STRING=${flags}"
      "-DCMAKE_PREFIX_PATH:STRING=${install_dir}"
      INSTALL_COMMAND ""
      EXCLUDE_FROM_ALL YES
      STEP_TARGETS build
      DEPENDS gif-engine-install
      ${console_access}
  )
  ensure_dependencies("${fuzzer}")
  ExternalProject_Add_Step(
      "${fuzzer}" echo
      COMMAND "${CMAKE_COMMAND}" --build <BINARY_DIR> --target echo
      DEPENDEES build
      ALWAYS YES
  )
endforeach()
