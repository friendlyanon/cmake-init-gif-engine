#pragma once

#include <gif_engine/result_code.h>

typedef struct gif_result {
  gif_result_code code;
  void* data;
} gif_result;
