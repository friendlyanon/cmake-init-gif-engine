#include "gif_engine/gif_engine.h"

#include <stddef.h>
#include <stdint.h>

#include "gif_engine/error.h"
#include "gif_engine/structs.h"

gif_result gif_parse(uint8_t* buffer,
                     size_t buffer_size,
                     gif_details* details,
                     gif_allocator allocator)
{
  (void)buffer;
  (void)buffer_size;
  (void)details;
  (void)allocator;

  return (gif_result) {
      .code = GIF_SUCCESS,
      .data = NULL,
  };
}

gif_result gif_decode(gif_details* details, gif_allocator allocator)
{
  (void)details;
  (void)allocator;

  return (gif_result) {
      .code = GIF_SUCCESS,
      .data = NULL,
  };
}

gif_result gif_free_details(gif_details* details, gif_deallocator deallocator)
{
  (void)details;
  (void)deallocator;

  return (gif_result) {
      .code = GIF_SUCCESS,
      .data = NULL,
  };
}
