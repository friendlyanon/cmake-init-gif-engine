#pragma once

#include <gif_engine/result_code.h>

#define TRY(expression) \
  do { \
    const gif_result_code result_code = (expression); \
    if (result_code != GIF_SUCCESS) { \
      return result_code; \
    } \
  } while (0)
