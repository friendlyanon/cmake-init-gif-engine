#pragma once

#include <stdint.h>

#include "gif_engine/gif_engine.h"

typedef struct gif_parse_state {
  uint8_t** current;
  const uint8_t* end;

  gif_details* details;
  gif_allocator allocator;

  void* data;
} gif_parse_state;
