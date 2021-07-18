#include <Windows.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "gif_mmap.h"

static const char* failed_function_name = NULL;

enum
{
  CLEANUP_SIZE = 2,
};

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

static bool allocate_cleanup_data(HANDLE** cleanup_data)
{
  HANDLE heap = GetProcessHeap();
  if (heap == NULL) {
    failed_function_name = "GetProcessHeap";
    return false;
  }

  void* pointer = HeapAlloc(heap, 0, sizeof(HANDLE) * CLEANUP_SIZE);
  if (pointer == NULL) {
    failed_function_name = "HeapAlloc";
    return false;
  }

  *cleanup_data = pointer;
  return true;
}

gif_mmap_span gif_mmap_allocate(const char* path_to_file)
{
  void* pointer = NULL;
  size_t size = 0;
  HANDLE* cleanup_data = NULL;
  failed_function_name = NULL;

  HANDLE file_handle;
  if (!create_file(&file_handle, path_to_file)) {
    goto exit_allocate;
  }

  HANDLE mapping_handle;
  if (!create_file_mapping(&mapping_handle, file_handle)) {
    goto cleanup_file_handle;
  }

  if (!map_view_of_file(&pointer, mapping_handle)) {
    goto cleanup_mapping_handle;
  }

  if (!allocate_cleanup_data(&cleanup_data)) {
    goto cleanup_file_view;
  }

  cleanup_data[0] = mapping_handle;
  cleanup_data[1] = file_handle;

  size = GetFileSize(file_handle, NULL);
  goto exit_allocate;

cleanup_file_view:
  UnmapViewOfFile(pointer);

cleanup_mapping_handle:
  CloseHandle(mapping_handle);

cleanup_file_handle:
  CloseHandle(file_handle);

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
          "%s failed with error %d: %s",
          failed_function_name,
          error_code,
          error_message);

  LocalFree(error_message);
}

void gif_mmap_deallocate(gif_mmap_span* span)
{
  UnmapViewOfFile(span->pointer);
  HANDLE* cleanup_data = &span->cleanup_data;
  CloseHandle(cleanup_data[0]);
  CloseHandle(cleanup_data[1]);
  /*
   * Possible leak if GetProcessHeap() fails, but the deallocate function is
   * called at the end of the tests anyway
   */
  HeapFree(GetProcessHeap(), 0, cleanup_data);
}
