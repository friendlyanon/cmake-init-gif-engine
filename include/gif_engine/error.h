#pragma once

typedef enum gif_result_code {
  GIF_SUCCESS,

  GIF_READ_PAST_BUFFER,

  GIF_ALLOC_FAIL,
  GIF_REALLOC_FAIL,
  GIF_DEALLOC_FAIL,

  GIF_NOT_A_GIF,
  GIF_NOT_A_GIF89A,

  GIF_UNKNOWN_BLOCK,
  GIF_UNKNOWN_EXTENSION,

  GIF_GRAPHICS_CONTROL_EXTENSION_SIZE_MISMATCH,
  GIF_MULTIPLE_GRAPHICS_CONTROL_EXTENSIONS,
  GIF_GRAPHICS_CONTROL_EXTENSION_NULL_MISSING,

  GIF_NOT_A_NETSCAPE_EXTENSION,
  GIF_NOT_A_NETSCAPE_20_EXTENSION,
  GIF_INCORRECT_NETSCAPE_SUBBLOCK_SIZE,
  GIF_INCORRECT_NETSCAPE_SUBBLOCK_ID,
  GIF_NETSCAPE_NULL_MISSING,
} gif_result_code;

typedef struct gif_result {
  gif_result_code code;
  void* data;
} gif_result;
