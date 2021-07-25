cmake_minimum_required(VERSION 3.14)

if(NOT CMAKE_VERSION VERSION_LESS "3.19")
  cmake_policy(SET CMP0114 NEW)
endif()

project(gif_engineFuzzSuperbuild LANGUAGES NONE)

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
)

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
  )
  ExternalProject_Add_Step(
      "${fuzzer}" echo
      COMMAND "${CMAKE_COMMAND}" --build <BINARY_DIR> --target echo
      DEPENDEES build
      ALWAYS YES
  )
endforeach()
