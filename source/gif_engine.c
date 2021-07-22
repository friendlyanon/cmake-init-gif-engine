#include "gif_engine/gif_engine.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "decode.h"
#include "last_position.h"
#include "parse.h"

gif_result gif_parse(uint8_t* buffer,
                     size_t buffer_size,
                     gif_details* details,
                     gif_allocator allocator)
{
  memset(details, 0, sizeof(gif_details));
  gif_parse_detail_set_globals(buffer, buffer_size, details, allocator);
  gif_set_last_position(NULL);

  void* data = NULL;
  gif_result_code code = gif_parse_impl(&data);
  gif_parse_detail_set_globals(NULL, 0, NULL, NULL);

  return (gif_result) {
      .code = code,
      .data = data,
  };
}

gif_result gif_decode(gif_details* details, gif_allocator allocator)
{
  void* data = NULL;
  gif_result_code code = gif_decode_impl(&data, details, allocator);

  return (gif_result) {
      .code = code,
      .data = data,
  };
}

static void free_frame_vector(gif_frame_vector frame_vector,
                              gif_deallocator deallocator)
{
  size_t size = frame_vector.size;
  if (size == 0) {
    return;
  }

  for (size_t i = 0; i < size; ++i) {
    deallocator(frame_vector.frames[i].local_color_table);
  }

  deallocator(frame_vector.frames);
}

void gif_free_details(gif_details* details, gif_deallocator deallocator)
{
  deallocator(details->global_color_table);
  free_frame_vector(details->frame_vector, deallocator);
}
