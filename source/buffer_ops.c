#include "buffer_ops.h"

#include <assert.h>
#include <string.h>

compare_result buffer_is_eq(uint8_t** begin,
                            uint8_t* end,
                            uint8_t* data,
                            size_t data_size)
{
  if ((size_t)(end - *begin) < data_size) {
    return CMP_OOB;
  }

  int result = memcmp(*begin, data, data_size);
  *begin += data_size;
  return result == 0 ? CMP_EQ : CMP_NEQ;
}

bool read_byte(uint8_t** begin, uint8_t* end, uint8_t* destination)
{
  if (end - *begin < 1) {
    return false;
  }

  *destination = read_byte_un(begin);
  return true;
}

uint8_t read_byte_un(uint8_t** buffer)
{
  uint8_t result;
  uint8_t* begin = *buffer;
  result = begin[0];
  ++*buffer;
  return result;
}

bool read_le_short(uint8_t** begin, uint8_t* end, uint16_t* destination)
{
  if (end - *begin < 2) {
    return false;
  }

  *destination = read_le_short_un(begin);
  return true;
}

uint16_t read_le_short_un(uint8_t** buffer)
{
  uint16_t result;
  uint8_t* begin = *buffer;
  result = begin[0] | begin[1] << 8;
  *buffer += 2;
  return result;
}

bool skip_bytes(uint8_t** begin, uint8_t* end, size_t count)
{
  if ((size_t)(end - *begin) < count) {
    return false;
  }

  *begin += count;
  return true;
}

bool read_bytes(uint8_t** begin, uint8_t* end, uint8_t* buffer, size_t count)
{
  if ((size_t)(end - *begin) < count) {
    return false;
  }

  memcpy(buffer, *begin, count);
  *begin += count;
  return true;
}

uint32_t read_color_un(uint8_t** buffer)
{
  uint8_t color_buffer[3];
  memcpy(color_buffer, *buffer, 3);
  *buffer += 3;
  return color_buffer[0] << 16 | color_buffer[1] << 8 | color_buffer[2];
}

static size_t size_to_count(uint8_t size)
{
  assert(size < 8U);
  return 2ULL << size;
}

gif_result_code read_color_table(uint8_t** begin,
                                 uint8_t* end,
                                 uint32_t** destination,
                                 uint8_t size,
                                 gif_allocator allocator)
{
  size_t color_count = size_to_count(size);
  size_t color_bytes = color_count * 3;
  if ((size_t)(end - *begin) < color_bytes) {
    return GIF_READ_PAST_BUFFER;
  }

  uint32_t* buffer = allocator(NULL, color_bytes * sizeof(uint32_t));
  if (buffer == NULL) {
    return GIF_ALLOC_FAIL;
  }

  for (size_t i = 0; i < color_count; ++i) {
    buffer[i] = read_color_un(begin);
  }

  *destination = buffer;
  return GIF_SUCCESS;
}
