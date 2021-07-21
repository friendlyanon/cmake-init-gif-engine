#include "parse.h"

#include <assert.h>
#include <string.h>

#include "buffer_ops.h"
#include "last_position.h"

static uint8_t* buffer_;
static size_t buffer_size_;
static gif_details* details_;
static gif_allocator allocator_;

void gif_parse_detail_set_globals(uint8_t* buffer,
                                  size_t buffer_size,
                                  gif_details* details,
                                  gif_allocator allocator)
{
  buffer_ = buffer;
  buffer_size_ = buffer_size;
  details_ = details;
  allocator_ = allocator;
}

static uint8_t magic[] = {'G', 'I', 'F'};

static compare_result is_gif_file(uint8_t** begin, uint8_t* end)
{
  return buffer_is_eq(begin, end, magic, sizeof(magic));
}

static uint8_t gif_version[] = {'8', '9', 'a'};

static compare_result is_gif_version_supported(uint8_t** begin, uint8_t* end)
{
  return buffer_is_eq(begin, end, gif_version, sizeof(gif_version));
}

enum { LOGICAL_SCREEN_DESCRIPTOR_SIZE = 7U };

static bool read_descriptor(uint8_t** begin, uint8_t* end)
{
  if (end - *begin < LOGICAL_SCREEN_DESCRIPTOR_SIZE) {
    return false;
  }

  gif_descriptor* descriptor = &details_->descriptor;
  descriptor->canvas_width = read_le_short_un(begin);
  descriptor->canvas_height = read_le_short_un(begin);

  uint8_t packed_byte = read_byte_un(begin);
  gif_descriptor_packed* packed = &descriptor->packed;
  packed->global_color_table_flag = (packed_byte & 0b1000'0000) != 0;
  packed->color_resolution = (packed_byte & 0b0111'0000) >> 4;
  packed->sort_flag = (packed_byte & 0b0000'1000) != 0;
  packed->size = packed_byte & 0b0000'0111;

  descriptor->background_color_index = read_byte_un(begin);
  descriptor->pixel_aspect_ratio = read_byte_un(begin);

  return true;
}

static size_t size_to_count(uint8_t size)
{
  assert(size < 8U);
  return 2ULL << size;
}

static gif_result_code read_global_color_table(uint8_t** begin, uint8_t* end)
{
  size_t color_count = size_to_count(details_->descriptor.packed.size);
  size_t color_bytes = color_count * 3;
  if ((size_t)(end - *begin) < color_bytes) {
    return GIF_READ_PAST_BUFFER;
  }

  uint32_t* buffer = allocator_(NULL, color_bytes * sizeof(uint32_t));
  if (buffer == NULL) {
    return GIF_ALLOC_FAIL;
  }

  for (size_t i = 0; i < color_count; ++i) {
    buffer[i] = read_color_un(begin);
  }

  details_->global_color_table = buffer;
  return GIF_SUCCESS;
}

typedef enum gif_extension_type {
  GIF_GRAPHICS_CONTROL_EXTENSION = 0xF9,
  GIF_APPLICATION_EXTENSION = 0xFF,
  GIF_COMMENT_EXTENSION = 0xFE,
  GIF_TEXT_EXTENSION = 0x01,
} gif_extension_type;

static gif_result_code read_extension_block(void** data,
                                            uint8_t** current,
                                            uint8_t* end)
{
  uint8_t extension_type_byte;
  if (!read_byte(current, end, &extension_type_byte)) {
    gif_set_last_position(*current);
    return GIF_READ_PAST_BUFFER;
  }

  gif_extension_type extension_type = (gif_block_type)extension_type_byte;
  switch (extension_type) {
    default:
      gif_set_last_position(*current);
      return GIF_UNKNOWN_EXTENSION;
  }
}

static gif_result_code read_image_descriptor_block(void** data,
                                                   uint8_t** current,
                                                   uint8_t* end)
{
  return GIF_SUCCESS;
}

typedef enum gif_block_type {
  GIF_EXTENSION_BLOCK = 0x21,
  GIF_IMAGE_DESCRIPTOR_BLOCK = 0x2C,
  GIF_TAIL_BLOCK = 0x3B,
} gif_block_type;

gif_result_code gif_parse_impl(void** data)
{
  uint8_t* current = buffer_;
  uint8_t* end = buffer_ + buffer_size_;

#define CONST_CHECK(predicate, fail_code) \
  do { \
    switch (predicate(&current, end)) { \
      case CMP_EQ: \
        break; \
      case CMP_NEQ: \
        return fail_code; \
      case CMP_OOB: \
        gif_set_last_position(current); \
        return GIF_READ_PAST_BUFFER; \
    } \
  } while (0)

  CONST_CHECK(is_gif_file, GIF_NOT_A_GIF);
  CONST_CHECK(is_gif_version_supported, GIF_NOT_A_GIF89A);

  if (!read_descriptor(&current, end)) {
    gif_set_last_position(current);
    return GIF_READ_PAST_BUFFER;
  }

  if (details_->descriptor.packed.global_color_table_flag) {
    gif_result_code code = read_global_color_table(&current, end);
    if (code != GIF_SUCCESS) {
      gif_set_last_position(current);
      return code;
    }
  }

  while (1) {
    uint8_t block_type_byte;
    if (!read_byte(&current, end, &block_type_byte)) {
      gif_set_last_position(current);
      return GIF_READ_PAST_BUFFER;
    }

    gif_block_type block_type = (gif_block_type)block_type_byte;
    switch (block_type) {
      case GIF_EXTENSION_BLOCK: {
        gif_result_code code = read_extension_block(data, &current, end);
        if (code != GIF_SUCCESS) {
          return code;
        }
        break;
      }
      case GIF_IMAGE_DESCRIPTOR_BLOCK: {
        gif_result_code code = read_image_descriptor_block(data, &current, end);
        if (code != GIF_SUCCESS) {
          return code;
        }
        break;
      }
      case GIF_TAIL_BLOCK:
        goto tail_block;
      default:
        gif_set_last_position(current);
        return GIF_UNKNOWN_BLOCK;
    }
  }

tail_block:
  _Static_assert(sizeof(void*) >= sizeof(size_t),
                 "void* should have a size greater than or equal to size_t");
  size_t leftover_bytes = end - current;
  memcpy(*data, &leftover_bytes, sizeof(size_t));

  return GIF_SUCCESS;
}
