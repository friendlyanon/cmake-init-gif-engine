#include "buffer_ops.h"

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
  *begin = *begin + data_size;
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
  *buffer = *buffer + 2;
  return result;
}
