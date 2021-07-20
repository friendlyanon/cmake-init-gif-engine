#include "buffer_ops.h"

#include <string.h>

compare_result is_eq_safe(uint8_t* begin,
                          uint8_t* end,
                          uint8_t* data,
                          size_t data_size)
{
  if (end - begin < data_size) {
    return CMP_OOB;
  }

  return memcmp(begin, data, data_size) == 0 ? CMP_EQ : CMP_NEQ;
}
