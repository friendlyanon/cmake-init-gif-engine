#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct gif_descriptor_packed {
  bool global_color_table_flag;

  uint8_t color_resolution;

  bool sort_flag;

  uint8_t size;
} gif_descriptor_packed;

typedef struct gif_descriptor {
  uint16_t canvas_width;
  uint16_t canvas_height;

  gif_descriptor_packed packed;

  uint8_t background_color_index;

  uint8_t pixel_aspect_ratio;
} gif_descriptor;

typedef enum gif_disposal_method {
  GIF_DISPOSAL_UNSPECIFIED = 0,
  GIF_DISPOSAL_NOTHING = 1,
  GIF_DISPOSAL_BACKGROUND = 2,
  GIF_DISPOSAL_PREVIOUS = 3,
} gif_disposal_method;

typedef struct gif_graphic_extension_packed {
  gif_disposal_method disposal_method;

  bool user_input_flag;

  bool transparent_color_flag;
} gif_graphic_extension_packed;

typedef struct gif_graphic_extension {
  gif_graphic_extension_packed packed;

  uint16_t delay;

  uint8_t transparent_color_index;
} gif_graphic_extension;

typedef struct gif_frame_descriptor_packed {
  bool local_color_table_flag;

  bool interlace_flag;

  bool sort_flag;

  uint8_t size;
} gif_frame_descriptor_packed;

typedef struct gif_frame_descriptor {
  uint16_t left;
  uint16_t top;
  uint16_t width;
  uint16_t height;

  gif_frame_descriptor_packed packed;
} gif_frame_descriptor;

typedef struct gif_frame_data {
  gif_graphic_extension graphic_extension;

  uint32_t* local_color_table;

  uint8_t min_code_size;

  gif_frame_descriptor descriptor;

  const uint8_t* first_subblock;
  size_t data_length;
} gif_frame_data;

typedef struct gif_frame_vector {
  gif_frame_data* frames;
  size_t size;
  size_t capacity;
} gif_frame_vector;

typedef struct gif_details {
  gif_descriptor descriptor;

  uint32_t* global_color_table;

  uint16_t repeat_count;

  gif_frame_vector frame_vector;

  const uint8_t* raw_data;
  size_t raw_data_size;
} gif_details;

typedef struct gif_frame_span {
  const uint32_t* data;
  size_t size;
} gif_frame_span;
