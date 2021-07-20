#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum compare_result {
  CMP_OOB,
  CMP_EQ,
  CMP_NEQ,
} compare_result;

compare_result is_eq_safe(uint8_t* begin,
                          uint8_t* end,
                          uint8_t* data,
                          size_t data_size);
