#pragma once

#include <gif_engine/result_code.h>

typedef struct gif_parse_result {
  gif_result_code code;
  const void* data;
  const void* last_position;
} gif_parse_result;

typedef struct gif_decode_result {
  gif_result_code code;
  const void* data;
} gif_decode_result;
