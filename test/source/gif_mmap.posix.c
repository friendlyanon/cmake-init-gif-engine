#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "gif_mmap.h"

static const char* failed_function_name = NULL;

gif_mmap_span gif_mmap_allocate(const char* path_to_file)
{
  void* pointer = NULL;
  size_t size = 0;
  void* cleanup_data = NULL;
  failed_function_name = NULL;

  int file_descriptor = open(path_to_file, O_RDONLY);
  if (file_descriptor == -1) {
    failed_function_name = "open";
    goto exit_allocate;
  }

  struct stat stat_object;
  if (fstat(file_descriptor, &stat_object) != 0) {
    failed_function_name = "fstat";
    goto cleanup_file_descriptor;
  }

  void* mapping =
      mmap(NULL, stat_object.st_size, PROT_READ, 0, file_descriptor, 0);
  if (mapping == MAP_FAILED) {
    failed_function_name = "mmap";
    goto cleanup_file_descriptor;
  }

  pointer = mapping;
  size = stat_object.st_size;
  memcpy(&cleanup_data, &file_descriptor, sizeof(int));
  goto exit_allocate;

cleanup_file_descriptor:
  close(file_descriptor);

exit_allocate:
  return (gif_mmap_span) {
      .pointer = pointer,
      .size = size,
      .cleanup_data = cleanup_data,
  };
}

void gif_mmap_print_last_error_to_stderr(void)
{
  if (failed_function_name == NULL) {
    return;
  }

  int error_code = errno;
  const char* error_message = strerror(error_code);
  fprintf(stderr,
          "%s failed with error %d: %s\n",
          failed_function_name,
          error_code,
          error_message);
}

bool gif_mmap_deallocate(gif_mmap_span* span)
{
  if (munmap(span->pointer, span->size) != 0) {
    return false;
  }

  int file_descriptor;
  memcpy(&file_descriptor, &span->cleanup_data, sizeof(int));
  return close(file_descriptor) == 0;
}
