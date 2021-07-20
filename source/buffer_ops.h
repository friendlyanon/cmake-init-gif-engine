#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum compare_result {
  CMP_OOB,
  CMP_EQ,
  CMP_NEQ,
} compare_result;

/**
 * A memcmp wrapper to compare begin and data until data_size length with bounds
 * checking.
 *
 * This function will advance the pointer pointed to by begin by data_size if
 * the read isn't OOB.
 *
 * @return Result of the equality comparison or an indication of OOB
 */
compare_result buffer_is_eq(uint8_t** begin,
                            uint8_t* end,
                            uint8_t* data,
                            size_t data_size);

/**
 * Reads an 8 bit number with bounds checking.
 *
 * This function will advance the pointer pointed to by begin by 1 if the read
 * isn't OOB.
 *
 * @return true if the read isn't OOB, otherwise false
 */
bool read_byte(uint8_t** begin, uint8_t* end, uint8_t* destination);

/**
 * Reads a 8 bit number without bounds checking.
 *
 * This function will advance the pointer pointed to by buffer by 1.
 */
uint8_t read_byte_un(uint8_t** buffer);

/**
 * Reads a 16 bit little-endian number with bounds checking.
 *
 * This function will advance the pointer pointed to by begin by 2 if the read
 * isn't OOB.
 *
 * @return true if the read isn't OOB, otherwise false
 */
bool read_le_short(uint8_t** begin, uint8_t* end, uint16_t* destination);

/**
 * Reads a 16 bit little-endian number without bounds checking.
 *
 * This function will advance the pointer pointed to by buffer by 2.
 */
uint16_t read_le_short_un(uint8_t** buffer);
