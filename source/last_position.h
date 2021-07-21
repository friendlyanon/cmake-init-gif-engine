#pragma once

void gif_set_last_position(void* data);

#define THROW(code, position) \
  do { \
    gif_set_last_position(position); \
    return code; \
  } while (0)
