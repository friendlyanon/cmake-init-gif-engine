#pragma once

#include <stddef.h>

typedef struct gif_mmap_span {
  void* pointer;
  size_t size;
} gif_mmap_span;

/**
 * @brief Maps the provided file into memory using OS specific functions
 *
 * @return A span, whose pointer member is null in case of an error
 */
gif_mmap_span gif_mmap_allocate(const char* path_to_file);

/**
 * @brief Prints the error that occured during the call to gif_mmap_allocate
 */
void gif_mmap_print_last_error_to_stderr(void);

/**
 * @brief Deallocates the file mapped to the location gif_mmap_allocate returns
 */
void gif_mmap_deallocate(void* pointer_to_allocation);
