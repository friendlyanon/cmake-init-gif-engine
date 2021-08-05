#include <Windows.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "gif_mmap.h"

static const char* failed_function_name = NULL;

static bool create_file(HANDLE* result_handle, const char* path_to_file)
{
  HANDLE handle = CreateFileA(path_to_file,
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);
  if (handle == INVALID_HANDLE_VALUE) {
    failed_function_name = "CreateFileA";
    return false;
  }

  *result_handle = handle;
  return true;
}

static bool create_file_mapping(HANDLE* mapping_handle, HANDLE file_handle)
{
  HANDLE handle =
      CreateFileMappingA(file_handle, NULL, PAGE_READONLY, 0, 0, NULL);
  if (handle == NULL) {
    failed_function_name = "CreateFileMappingA";
    return false;
  }

  *mapping_handle = handle;
  return true;
}

static bool map_view_of_file(void** pointer, HANDLE mapping_handle)
{
  void* mapping = MapViewOfFile(mapping_handle, FILE_MAP_READ, 0, 0, 0);
  if (mapping == NULL) {
    failed_function_name = "MapViewOfFile";
    return false;
  }

  *pointer = mapping;
  return true;
}

gif_mmap_span gif_mmap_allocate(const char* path_to_file)
{
  void* pointer = NULL;
  size_t size = 0;
  failed_function_name = NULL;

  HANDLE file_handle = INVALID_HANDLE_VALUE;
  HANDLE mapping_handle = INVALID_HANDLE_VALUE;
  if (!create_file(&file_handle, path_to_file)) {
    goto exit_allocate;
  }

  if (!create_file_mapping(&mapping_handle, file_handle)) {
    goto cleanup_file_handle;
  }

  if (!map_view_of_file(&pointer, mapping_handle)) {
    goto cleanup_mapping_handle;
  }

  size = GetFileSize(file_handle, NULL);
  goto exit_allocate;

cleanup_mapping_handle:
  CloseHandle(mapping_handle);

cleanup_file_handle:
  CloseHandle(file_handle);

exit_allocate:
  return (gif_mmap_span) {
      .pointer = pointer,
      .size = size,
      .cleanup_data = {mapping_handle, file_handle},
  };
}

void gif_mmap_print_last_error_to_stderr(void)
{
  if (failed_function_name == NULL) {
    return;
  }

  DWORD error_code = GetLastError();

  char* error_message;
  FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
                     | FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL,
                 error_code,
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                 (char*)&error_message,
                 0,
                 NULL);

  fprintf(stderr,
          "%s failed with error %d: %s\n",
          failed_function_name,
          error_code,
          error_message);

  LocalFree(error_message);
}

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(*array))

bool gif_mmap_deallocate(gif_mmap_span* span)
{
  if (UnmapViewOfFile(span->pointer) == 0) {
    return false;
  }

  HANDLE* cleanup_data = span->cleanup_data;
  for (size_t i = 0; i < ARRAY_LENGTH(span->cleanup_data); ++i) {
    HANDLE handle = cleanup_data[i];
    if (handle != INVALID_HANDLE_VALUE && CloseHandle(handle) == 0) {
      return false;
    }
  }

  return true;
}
