#include "parse.h"

#include "buffer_ops.h"

static uint8_t* buffer_;
static size_t buffer_size_;
static gif_details* details_;
static gif_allocator allocator_;

void gif_parse_detail_set_globals(uint8_t* buffer,
                                  size_t buffer_size,
                                  gif_details* details,
                                  gif_allocator allocator)
{
  (void)buffer;
  (void)buffer_size;
  (void)details;
  (void)allocator;
}

gif_result_code gif_parse_impl(void** data)
{
  (void)data;

  return GIF_SUCCESS;
}
