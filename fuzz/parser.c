#include <gif_engine/gif_engine.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

int LLVMFuzzerTestOneInput(uint8_t* data, size_t size)
{
  if (size == 0) {
    return 0;
  }

  gif_details details;
  gif_parse(data, size, &details, &realloc);
  gif_free_details(&details, &free);
  return 0;
}
