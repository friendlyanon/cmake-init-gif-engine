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

static bool skip_block(uint8_t** current, uint8_t* end)
{
  while (1) {
    uint8_t subblock_size;
    if (!read_byte(current, end, &subblock_size)) {
      return false;
    }
    if (subblock_size == 0) {
      return true;
    }
    if (!skip_bytes(current, end, subblock_size)) {
      return false;
    }
  }
}

enum { GIF_FRAME_VECTOR_GROWTH = 10U };

static gif_result_code ensure_frame_data(size_t frame_index)
{
  gif_frame_vector* frame_vector = &details_->frame_vector;

  /* Sanity check: ensure_frame_data should be called with monotonically
   * non-decreasing indexes */
  size_t capacity = frame_vector->capacity;
  assert(frame_index <= capacity);

  if (capacity == frame_index) {
    size_t new_capacity = capacity + GIF_FRAME_VECTOR_GROWTH;
    size_t byte_length = sizeof(gif_frame_data) * new_capacity;
    gif_frame_data* frames_allocation =
        allocator_(frame_vector->frames, byte_length);
    if (frames_allocation == NULL) {
      return frame_vector->frames == NULL ? GIF_ALLOC_FAIL : GIF_REALLOC_FAIL;
    }

    frame_vector->capacity = new_capacity;
    frame_vector->frames = frames_allocation;
  }

  /* Same sanity check as above */
  size_t size = frame_vector->size;
  assert(frame_index == size || frame_index == size - 1);

  if (frame_index == size) {
    memset(&frame_vector->frames[size], 0, sizeof(gif_frame_data));
    frame_vector->size = size + 1;
  }

  return GIF_SUCCESS;
}

enum { GIF_GRAPHICS_CONTROL_EXTENSION_SIZE = 4U };

static gif_result_code read_graphics_control_extension(void** data,
                                                       uint8_t** current,
                                                       uint8_t* end,
                                                       size_t frame_index)
{
  (void)data;

  /* The plus two comes from the length byte itself and the terminating null
   * byte */
  if ((size_t)(end - *current) < GIF_GRAPHICS_CONTROL_EXTENSION_SIZE + 2U) {
    return GIF_READ_PAST_BUFFER;
  }

  gif_result_code frame_data_code = ensure_frame_data(frame_index);
  if (frame_data_code != GIF_SUCCESS) {
    return frame_data_code;
  }

  if (read_byte_un(current) != GIF_GRAPHICS_CONTROL_EXTENSION_SIZE) {
    return GIF_GRAPHICS_CONTROL_EXTENSION_SIZE_MISMATCH;
  }

  uint8_t packed_byte = read_byte_un(current);
  uint16_t delay = read_le_short_un(current);
  uint8_t transparent_color_index = read_byte_un(current);
  uint8_t terminator = read_byte_un(current);
  if (terminator != 0) {
    return GIF_GRAPHICS_CONTROL_EXTENSION_NULL_MISSING;
  }

  gif_graphic_extension* graphic_extension =
      &details_->frame_vector.frames[frame_index].graphic_extension;
  gif_graphic_extension_packed* packed = &graphic_extension->packed;
  packed->disposal_method = (packed_byte & 0b0001'1100) >> 2;
  packed->user_input_flag = (packed_byte & 0b0000'0010) != 0;
  packed->transparent_color_flag = packed_byte & 0b0000'0001;

  graphic_extension->delay = delay;
  graphic_extension->transparent_color_index = transparent_color_index;

  return GIF_SUCCESS;
}

enum {
    GIF_APPLICATION_EXTENSION_SIZE = 11U,
    GIF_APPLICATION_IDENTIFIER_SIZE = 8U,
    GIF_APPLICATION_AUTH_CODE_SIZE = 3U,
    GIF_NETSCAPE_SUBBLOCK_SIZE = 3U,
    GIF_NETSCAPE_SUBBLOCK_ID = 1U,
};

static uint8_t netscape_identifier[GIF_APPLICATION_IDENTIFIER_SIZE] = {
    'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E'};

static uint8_t netscape_auth_code[GIF_APPLICATION_AUTH_CODE_SIZE] = {
    '2', '.', '0'};

static gif_result_code read_application_extension(uint8_t** current,
                                                  uint8_t* end)
{
  /* The plus two comes from the length byte itself and the subblock length
   * byte, which could potentially be the terminating null byte */
  if ((size_t)(end - *current) < GIF_APPLICATION_EXTENSION_SIZE + 2U) {
    return GIF_READ_PAST_BUFFER;
  }

  bool is_netscape_extension =
      memcmp(*current, netscape_identifier, GIF_APPLICATION_IDENTIFIER_SIZE)
      != 0;
  if (!is_netscape_extension) {
    return GIF_NOT_A_NETSCAPE_EXTENSION;
  }
  *current += GIF_APPLICATION_IDENTIFIER_SIZE;

  bool is_netscape_20_extension =
      memcmp(*current, netscape_auth_code, GIF_APPLICATION_AUTH_CODE_SIZE) != 0;
  if (!is_netscape_20_extension) {
    return GIF_NOT_A_NETSCAPE_20_EXTENSION;
  }
  *current += GIF_APPLICATION_AUTH_CODE_SIZE;

  uint8_t subblock_length = read_byte_un(current);
  if (subblock_length != GIF_NETSCAPE_SUBBLOCK_SIZE) {
    return GIF_INCORRECT_NETSCAPE_SUBBLOCK_SIZE;
  }

  uint8_t subblock_id;
  if (!read_byte(current, end, &subblock_id)) {
    return GIF_READ_PAST_BUFFER;
  }

  if (subblock_id != GIF_NETSCAPE_SUBBLOCK_ID) {
    return GIF_INCORRECT_NETSCAPE_SUBBLOCK_ID;
  }

  uint16_t repeat_count;
  if (!read_le_short(current, end, &repeat_count)) {
    return GIF_READ_PAST_BUFFER;
  }

  uint8_t terminator_byte;
  if (!read_byte(current, end, &terminator_byte)) {
    return GIF_READ_PAST_BUFFER;
  }

  if (terminator_byte != 0) {
    return GIF_NETSCAPE_NULL_MISSING;
  }

  details_->repeat_count = repeat_count;
  return GIF_SUCCESS;
}

static gif_result_code read_extension_block(
    void** data,
    uint8_t** current,
    uint8_t* end,
    size_t frame_index,
    bool* seen_graphics_control_extension)
{
  uint8_t extension_type_byte;
  if (!read_byte(current, end, &extension_type_byte)) {
    THROW(GIF_READ_PAST_BUFFER, *current);
  }

  gif_extension_type extension_type = (gif_extension_type)extension_type_byte;
  switch (extension_type) {
    case GIF_GRAPHICS_CONTROL_EXTENSION: {
      if (*seen_graphics_control_extension) {
        THROW(GIF_MULTIPLE_GRAPHICS_CONTROL_EXTENSIONS, *current);
      }
      *seen_graphics_control_extension = true;

      gif_result_code code =
          read_graphics_control_extension(data, current, end, frame_index);
      if (code != GIF_SUCCESS) {
        THROW(code, *current);
      }
      break;
    }
    case GIF_APPLICATION_EXTENSION: {
      gif_result_code code = read_application_extension(current, end);
      if (code != GIF_SUCCESS) {
        THROW(code, *current);
      }
      break;
    }
    case GIF_COMMENT_EXTENSION:
      /* fallthrough */
    case GIF_TEXT_EXTENSION:
      if (!skip_block(current, end)) {
        THROW(GIF_READ_PAST_BUFFER, *current);
      }
      break;
    default:
      THROW(GIF_UNKNOWN_EXTENSION, *current);
  }

  return GIF_SUCCESS;
}

static gif_result_code read_image_descriptor_block(void** data,
                                                   uint8_t** current,
                                                   uint8_t* end,
                                                   size_t frame_index)
{
  (void)data;
  (void)current;
  (void)end;
  (void)frame_index;

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
    THROW(GIF_READ_PAST_BUFFER, current);
  }

  if (details_->descriptor.packed.global_color_table_flag) {
    gif_result_code code = read_global_color_table(&current, end);
    if (code != GIF_SUCCESS) {
      THROW(code, current);
    }
  }

  size_t frame_index = 0;
  bool seen_graphics_control_extension = false;
  while (1) {
    uint8_t block_type_byte;
    if (!read_byte(&current, end, &block_type_byte)) {
      THROW(GIF_READ_PAST_BUFFER, current);
    }

    gif_block_type block_type = (gif_block_type)block_type_byte;
    switch (block_type) {
      case GIF_EXTENSION_BLOCK: {
        gif_result_code code = read_extension_block(
            data, &current, end, frame_index, &seen_graphics_control_extension);
        if (code != GIF_SUCCESS) {
          return code;
        }
        break;
      }
      case GIF_IMAGE_DESCRIPTOR_BLOCK: {
        gif_result_code code =
            read_image_descriptor_block(data, &current, end, frame_index);
        if (code != GIF_SUCCESS) {
          return code;
        }
        ++frame_index;
        seen_graphics_control_extension = false;
        break;
      }
      case GIF_TAIL_BLOCK:
        if (frame_index == 0) {
          THROW(GIF_IMAGE_DESCRIPTOR_MISSING, current);
        }
        goto tail_block;
      default:
        THROW(GIF_UNKNOWN_BLOCK, current);
    }
  }

tail_block:
  _Static_assert(sizeof(void*) >= sizeof(size_t),
                 "void* should have a size greater than or equal to size_t");
  size_t leftover_bytes = end - current;
  memcpy(*data, &leftover_bytes, sizeof(size_t));

  return GIF_SUCCESS;
}
