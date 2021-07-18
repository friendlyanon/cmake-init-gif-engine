#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct gif_mmap_span {
  void* pointer;
  size_t size;
  void* cleanup_data;
} gif_mmap_span;

/**
 * Maps the provided file into memory using OS specific functions.
 *
 * @return A span, whose pointer member is null in case of an error
 */
gif_mmap_span gif_mmap_allocate(const char* path_to_file);

/**
 * Prints the error that occurred during the call to gif_mmap_allocate.
 */
void gif_mmap_print_last_error_to_stderr(void);

/**
 * Deallocates the mapping to the file mapped by gif_mmap_allocate.
 *
 * @return \c true if deallocation was successful, otherwise \c false . If this
 * function fails, it's advised to crash the program.
 */
bool gif_mmap_deallocate(gif_mmap_span* span);
