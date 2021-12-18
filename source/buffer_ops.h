#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gif_engine/gif_engine.h"

typedef enum compare_result {
  CMP_OOB,
  CMP_EQ,
  CMP_NEQ,
} compare_result;

/**
 * A \c memcmp wrapper to compare \c current and \c data until \c data_size
 * length with bounds checking. This function will advance the pointer pointed
 * to by \c current by \c data_size if the read isn't OOB.
 *
 * @return Result of the equality comparison or an indication of OOB
 */
compare_result buffer_is_eq(uint8_t** current,
                            size_t* remaining,
                            uint8_t* data,
                            size_t data_size);

/**
 * Reads an 8 bit number with bounds checking. This function will advance the
 * pointer pointed to by \c current by 1 if the read isn't OOB.
 *
 * @return \c true if the read isn't OOB, otherwise \c false
 */
bool read_byte(uint8_t** current, size_t* remaining, uint8_t* destination);

/**
 * Reads an 8 bit number without bounds checking. This function will advance
 * the pointer pointed to by \c buffer by 1.
 */
uint8_t read_byte_un(uint8_t** buffer);

/**
 * Reads a 16 bit little-endian number with bounds checking. This function will
 * advance the pointer pointed to by \c current by 2 if the read isn't OOB.
 *
 * @return \c true if the read isn't OOB, otherwise \c false
 */
bool read_le_short(uint8_t** current, size_t* remaining, uint16_t* destination);

/**
 * Reads a 16 bit little-endian number without bounds checking. This function
 * will advance the pointer pointed to by \c buffer by 2.
 */
uint16_t read_le_short_un(uint8_t** buffer);

/**
 * Skips \c count number of bytes with bounds checking. This function will
 * advance the pointer pointed to by \c current by \c count.
 */
bool skip_bytes(uint8_t** current, size_t* remaining, size_t count);

/**
 * Reads a 3 byte color in the RGB format. This function will advance the
 * pointer pointed to by \c current by 3.
 *
 * @return 32 bit integer in the \c 0x00RRGGBB format that encodes an RGB color
 */
uint32_t read_color_un(uint8_t** buffer);

/**
 * Reads a color table. This function will advance the pointer pointed to by \c
 * current by <tt>(2 &lt;&lt; size) * 3</tt>. The allocated color table will be
 * output via the \c destination parameter.
 */
gif_result_code read_color_table(uint8_t** current,
                                 size_t* remaining,
                                 uint32_t** destination,
                                 uint8_t size,
                                 gif_allocator allocator);
