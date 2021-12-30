#include "gif_engine/gif_engine.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "decode/decode.h"
#include "parse/parse.h"
#include "parse/parse_state.h"

gif_parse_result gif_parse(const void* buffer,
                           size_t buffer_size,
                           gif_details* const details,
                           const gif_allocator allocator)
{
  memset(details, 0, sizeof(gif_details));
  if (buffer_size == 0) {
    return (gif_parse_result) {.code = GIF_ZERO_SIZED_BUFFER};
  }

  gif_parse_state state = {
      .current = (const uint8_t**)&buffer,
      .remaining = &buffer_size,
      .details = details,
      .allocator = allocator,
      .data = NULL,
  };

  const gif_result_code code = gif_parse_impl(&state);

  return (gif_parse_result) {
      .code = code,
      .data = state.data,
      .last_position = code == GIF_SUCCESS ? NULL : buffer,
  };
}

gif_decode_result gif_decode(gif_details* const details,
                             const gif_allocator allocator)
{
  void* data = NULL;
  gif_result_code code = gif_decode_impl(&data, details, allocator);

  return (gif_decode_result) {
      .code = code,
      .data = data,
  };
}

static void free_frame_vector(const gif_frame_vector frame_vector,
                              const gif_deallocator deallocator)
{
  const size_t size = frame_vector.size;
  if (size == 0) {
    return;
  }

  for (size_t i = 0; i < size; ++i) {
    deallocator(frame_vector.frames[i].local_color_table);
  }

  deallocator(frame_vector.frames);
}

void gif_free_details(const gif_details* const details,
                      const gif_deallocator deallocator)
{
  deallocator(details->global_color_table);
  free_frame_vector(details->frame_vector, deallocator);
}
