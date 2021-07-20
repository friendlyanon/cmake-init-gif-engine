#pragma once

typedef enum gif_result_code {
  GIF_SUCCESS,

  GIF_READ_PAST_BUFFER,

  GIF_ALLOC_FAIL,
  GIF_DEALLOC_FAIL,

  GIF_NOT_A_GIF,
  GIF_NOT_A_GIF89A,

  GIF_UNKNOWN_BLOCK,
} gif_result_code;

typedef struct gif_result {
  gif_result_code code;
  void* data;
} gif_result;
