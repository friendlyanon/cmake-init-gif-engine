#pragma once

/**
 * Creates an unsigned hex literal from \c n.
 */
#define HEX_LITERAL(n) 0x##n##U

/* clang-format off */

/**
 * Builds a number that represents an unsigned 8-bit integer using masks.
 */
#define B8_BIN_MASK(x) ( \
  (((x) & 0x00000001U) ? 1 : 0) + \
  (((x) & 0x00000010U) ? 2 : 0) + \
  (((x) & 0x00000100U) ? 4 : 0) + \
  (((x) & 0x00001000U) ? 8 : 0) + \
  (((x) & 0x00010000U) ? 16 : 0) + \
  (((x) & 0x00100000U) ? 32 : 0) + \
  (((x) & 0x01000000U) ? 64 : 0) + \
  (((x) & 0x10000000U) ? 128 : 0))

/* clang-format on */

/**
 * Creates an unsigned 8-bit integer from the mask argument. Use it like so:
 *
 * \code{.c}
 * uint8_t mask = B8(10101010);
 * \endcode
 */
#define B8(mask) ((unsigned char)B8_BIN_MASK(HEX_LITERAL(mask)))
