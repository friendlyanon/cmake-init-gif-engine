cmake_minimum_required(VERSION 3.14)

# This script extracts the enumerations of the gif_result_code enum and writes
# a source file to the binary directory that implements stringification of this
# enum. This is done this way, because Doxygen cannot handle X macros, so it
# can't be used to implement.

file(STRINGS "${input}" lines REGEX ",$")

set(source [[
/* Generated file, do not edit! */

#include "gif_engine/gif_engine.h"
#include "stddef.h"

const char* gif_result_code_to_string(gif_result_code code)
{
  switch (code) {
]])
foreach(line IN LISTS lines)
  if(NOT line MATCHES "^  ([A-Z0-9_]+),")
    continue()
  endif()
  set(name "${CMAKE_MATCH_1}")
  string(APPEND source "    case ${name}:\n      return \"${name}\";\n")
endforeach()
string(APPEND source "  }\n\n  return NULL;\n}\n")

file(WRITE "${output}.tmp" "${source}")
