#pragma once

#include <gif_engine/error.h>
#include <gif_engine/export.h>
#include <gif_engine/structs.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** An allocator function type matching that of \c realloc. */
typedef void* (*gif_allocator)(void* pointer, size_t size);

/** A deallocator function type matching that of \c free. */
typedef void (*gif_deallocator)(void* allocation);

/**
 * Parses the GIF file located at \c buffer. This function will parse the
 * contents of \c buffer in a way that makes OOB reads impossible, if the
 * provided \c buffer and \c buffer_size arguments are valid. Because this
 * function must allocate, the user can provide an allocator function using the
 * \c allocator arguments, which is a pointer to a function with a signature
 * matching that of \c realloc. The \c details argument is a pointer to a
 * gif_result struct, which need not be zero initialized, because this function
 * does zero initialization as its first step.
 * Typical usage of this function looks as follows:
 *
 * \code{.c}
 * gif_details details;
 * gif_result result = gif_parse(buffer, buffer_size, &details, &realloc);
 * \endcode
 *
 * The \c code member of the returned gif_result object holds the result of
 * the parsing process. If the \c code is ::GIF_SUCCESS, then the \c data
 * member will hold the number of bytes remaining after the tail block in the
 * buffer. Note that the number of bytes is a \c size_t value memcpy'd into
 * this field, so you have to copy it out in a similar manner:
 *
 * \code{.c}
 * size_t leftover_bytes;
 * memcpy(&leftover_bytes, &result.data, sizeof(size_t));
 * \endcode
 *
 * If the \c code member is ::GIF_REALLOC_FAIL, then the pointer that caused
 * the \c allocator function to fail in a realloc context (i.e. when the first
 * argument isn't \c NULL) will be returned in the \c data member as is.
 *
 * This function is not thread-safe.
 */
GIF_ENGINE_EXPORT gif_result gif_parse(uint8_t* buffer,
                                       size_t buffer_size,
                                       gif_details* details,
                                       gif_allocator allocator);

/**
 * Returns the last position that was read in the buffer. This function will
 * pop an internal slot, which holds a pointer pointing into the memory region
 * provided as an argument to ::gif_parse. If that function didn't return
 * ::GIF_SUCCESS, then this function can be used to inspect the cause of the
 * failure.
 *
 * This function is not thread-safe.
 */
GIF_ENGINE_EXPORT void* gif_pop_last_position(void);

/**
 * Decodes the frame data parsed by ::gif_parse. This function is not yet
 * implemented.
 */
GIF_ENGINE_EXPORT gif_result gif_decode(gif_details* details,
                                        gif_allocator allocator);

/**
 * Frees the gif_details struct populated by ::gif_parse. This function should
 * be called even if the ::gif_parse function did not succeed.
 */
GIF_ENGINE_EXPORT void gif_free_details(gif_details* details,
                                        gif_deallocator deallocator);

/**
 * Returns the string representation of ::gif_result_code values. The returned
 * value will be \c NULL for unknown values.
 */
GIF_ENGINE_EXPORT const char* gif_result_code_to_string(gif_result_code code);

#ifdef __cplusplus
}  // extern "C"
#endif
