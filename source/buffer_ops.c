#include "buffer_ops.h"

#include <assert.h>
#include <string.h>

compare_result buffer_is_eq(const uint8_t** const current,
                            size_t* const remaining,
                            const uint8_t* data,
                            const size_t data_size)
{
  if (*remaining < data_size) {
    return CMP_OOB;
  }

  int result = memcmp(*current, data, data_size);
  *remaining -= data_size;
  *current += data_size;
  return result == 0 ? CMP_EQ : CMP_NEQ;
}

bool read_byte(const uint8_t** const current,
               size_t* const remaining,
               uint8_t* const destination)
{
  if (*remaining == 0) {
    return false;
  }

  --*remaining;
  *destination = read_byte_un(current);
  return true;
}

uint8_t read_byte_un(const uint8_t** const buffer)
{
  uint8_t byte = **buffer;
  ++*buffer;
  return byte;
}

bool read_le_short(const uint8_t** const current,
                   size_t* const remaining,
                   uint16_t* const destination)
{
  if (*remaining < 2U) {
    return false;
  }

  *remaining -= 2U;
  *destination = read_le_short_un(current);
  return true;
}

uint16_t read_le_short_un(const uint8_t** const buffer)
{
  const uint8_t* const pointer = *buffer;
  const uint16_t low_byte = pointer[0];
  const uint16_t high_byte = pointer[1];
  *buffer += 2;
  return (uint16_t)(high_byte << 8U | low_byte);
}

bool skip_bytes(const uint8_t** const current,
                size_t* const remaining,
                const size_t count)
{
  if (*remaining < count) {
    return false;
  }

  *remaining -= count;
  *current += count;
  return true;
}

uint32_t read_color_un(const uint8_t** const buffer)
{
  uint8_t color_buffer[3];
  memcpy(color_buffer, *buffer, 3);
  *buffer += 3;
  return (uint32_t)color_buffer[0] << 16U | (uint32_t)color_buffer[1] << 8U
      | (uint32_t)color_buffer[2];
}

static size_t size_to_count(const uint8_t size)
{
  assert(size < 8U);
  return 2ULL << size;
}

gif_result_code read_color_table(const uint8_t** const current,
                                 size_t* const remaining,
                                 uint32_t** const destination,
                                 const uint8_t size,
                                 gif_allocator allocator)
{
  const size_t color_count = size_to_count(size);
  const size_t color_bytes = color_count * 3;
  if (*remaining < color_bytes) {
    return GIF_READ_PAST_BUFFER;
  }

  uint32_t* const buffer = allocator(NULL, color_bytes * sizeof(uint32_t));
  if (buffer == NULL) {
    return GIF_ALLOC_FAIL;
  }

  for (size_t i = 0; i < color_count; ++i) {
    buffer[i] = read_color_un(current);
  }

  *remaining -= color_bytes;
  *destination = buffer;
  return GIF_SUCCESS;
}
