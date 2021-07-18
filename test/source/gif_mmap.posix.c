#include "gif_mmap.h"

gif_mmap_span gif_mmap_allocate(const char* path_to_file)
{
  (void)path_to_file;

  return (gif_mmap_span) {0};
}

void gif_mmap_print_last_error_to_stderr(void) {}

void gif_mmap_deallocate(gif_mmap_span* span)
{
  (void)span;
}
