#include "parse/parse.h"

#include <assert.h>
#include <string.h>

#include "binary_literal.h"
#include "buffer_ops.h"
#include "try.h"

static uint8_t magic[] = {'G', 'I', 'F'};

static compare_result is_gif_file(gif_parse_state* state)
{
  return buffer_is_eq(state->current, state->remaining, magic, sizeof(magic));
}

static uint8_t gif_version[] = {'8', '9', 'a'};

static compare_result is_gif_version_supported(gif_parse_state* state)
{
  return buffer_is_eq(
      state->current, state->remaining, gif_version, sizeof(gif_version));
}

#define LOGICAL_SCREEN_DESCRIPTOR_SIZE 7U

static bool read_descriptor(gif_parse_state* state)
{
  if (*state->remaining < LOGICAL_SCREEN_DESCRIPTOR_SIZE) {
    return false;
  }

  gif_descriptor* descriptor = &state->details->descriptor;
  descriptor->canvas_width = read_le_short_un(state->current);
  descriptor->canvas_height = read_le_short_un(state->current);

  uint8_t packed_byte = read_byte_un(state->current);
  gif_descriptor_packed* packed = &descriptor->packed;
  packed->global_color_table_flag = (packed_byte & B8(10000000)) != 0;
  packed->color_resolution = (packed_byte & B8(01110000)) >> 4U;
  packed->sort_flag = (packed_byte & B8(00001000)) != 0;
  packed->size = packed_byte & B8(00000111);

  descriptor->background_color_index = read_byte_un(state->current);
  descriptor->pixel_aspect_ratio = read_byte_un(state->current);

  *state->remaining -= LOGICAL_SCREEN_DESCRIPTOR_SIZE;
  return true;
}

typedef enum gif_extension_type {
  GIF_GRAPHICS_CONTROL_EXTENSION = 0xF9,
  GIF_APPLICATION_EXTENSION = 0xFF,
  GIF_COMMENT_EXTENSION = 0xFE,
  GIF_TEXT_EXTENSION = 0x01,
} gif_extension_type;

static bool skip_block(uint8_t** current, const uint8_t* end)
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

#define GIF_FRAME_VECTOR_GROWTH 10U

static gif_result_code ensure_frame_data(gif_parse_state* state,
                                         size_t frame_index)
{
  gif_frame_vector* frame_vector = &state->details->frame_vector;

  /* Sanity check: ensure_frame_data should be called with monotonically
   * non-decreasing indexes */
  size_t capacity = frame_vector->capacity;
  assert(frame_index <= capacity);

  if (capacity == frame_index) {
    size_t new_capacity = capacity + GIF_FRAME_VECTOR_GROWTH;
    size_t byte_length = sizeof(gif_frame_data) * new_capacity;
    gif_frame_data* frames_allocation =
        state->allocator(frame_vector->frames, byte_length);
    if (frames_allocation == NULL) {
      if (frame_vector->frames == NULL) {
        return GIF_ALLOC_FAIL;
      }

      state->data = frame_vector->frames;
      return GIF_REALLOC_FAIL;
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

#define GIF_GRAPHICS_CONTROL_EXTENSION_SIZE 4U

static gif_result_code read_graphics_control_extension(gif_parse_state* state,
                                                       size_t frame_index)
{
  /* The plus two comes from the length byte itself and the terminating null
   * byte */
  if (*state->remaining < GIF_GRAPHICS_CONTROL_EXTENSION_SIZE + 2U) {
    return GIF_READ_PAST_BUFFER;
  }

  TRY(ensure_frame_data(state, frame_index));

  if (read_byte_un(state->current) != GIF_GRAPHICS_CONTROL_EXTENSION_SIZE) {
    return GIF_GRAPHICS_CONTROL_EXTENSION_SIZE_MISMATCH;
  }

  uint8_t packed_byte = read_byte_un(state->current);
  uint16_t delay = read_le_short_un(state->current);
  uint8_t transparent_color_index = read_byte_un(state->current);
  uint8_t terminator = read_byte_un(state->current);
  if (terminator != 0) {
    return GIF_GRAPHICS_CONTROL_EXTENSION_NULL_MISSING;
  }

  gif_graphic_extension* graphic_extension =
      &state->details->frame_vector.frames[frame_index].graphic_extension;
  gif_graphic_extension_packed* packed = &graphic_extension->packed;
  uint8_t disposal_method = (packed_byte & B8(00011100)) >> 2U;
  packed->user_input_flag = (packed_byte & B8(00000010)) != 0;
  packed->transparent_color_flag = packed_byte & B8(00000001);

  if (disposal_method > 3U) {
    return GIF_UNKNOWN_DISPOSAL_METHOD;
  }
  packed->disposal_method = (gif_disposal_method)disposal_method;

  graphic_extension->delay = delay;
  graphic_extension->transparent_color_index = transparent_color_index;

  *state->remaining -= GIF_GRAPHICS_CONTROL_EXTENSION_SIZE + 2U;
  return GIF_SUCCESS;
}

static uint8_t netscape_identifier[] = {'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E'};

_Static_assert(sizeof(netscape_identifier) == 8U,
               "The application identifier must be 8 bytes long");

static uint8_t netscape_auth_code[] = {'2', '.', '0'};

_Static_assert(sizeof(netscape_auth_code) == 3U,
               "The application auth code must be 3 bytes long");

#define GIF_APPLICATION_EXTENSION_SIZE 11U
#define GIF_NETSCAPE_SUBBLOCK_SIZE 3U
#define GIF_NETSCAPE_SUBBLOCK_ID 1U

static gif_result_code read_application_extension(gif_parse_state* state)
{
  /* The plus two comes from the length byte itself and the subblock length
   * byte, which could potentially be the terminating null byte */
  if (*state->remaining < GIF_APPLICATION_EXTENSION_SIZE + 2U) {
    return GIF_READ_PAST_BUFFER;
  }

  if (read_byte_un(state->current) != GIF_APPLICATION_EXTENSION_SIZE) {
    return GIF_APPLICATION_EXTENSION_SIZE_MISMATCH;
  }

#define CONST_CHECK_UN(buffer, code) \
  do { \
    if (memcmp(*state->current, buffer, sizeof(buffer)) != 0) { \
      return code; \
    } \
    *state->current += sizeof(buffer); \
  } while (0)

  CONST_CHECK_UN(netscape_identifier, GIF_NOT_A_NETSCAPE_EXTENSION);
  CONST_CHECK_UN(netscape_auth_code, GIF_NOT_A_NETSCAPE_20_EXTENSION);

  uint8_t subblock_length = read_byte_un(state->current);
  if (subblock_length != GIF_NETSCAPE_SUBBLOCK_SIZE) {
    return GIF_INCORRECT_NETSCAPE_SUBBLOCK_SIZE;
  }

  uint8_t subblock_id;
  if (!read_byte(state->current, state->remaining, &subblock_id)) {
    return GIF_READ_PAST_BUFFER;
  }

  if (subblock_id != GIF_NETSCAPE_SUBBLOCK_ID) {
    return GIF_INCORRECT_NETSCAPE_SUBBLOCK_ID;
  }

  uint16_t repeat_count;
  if (!read_le_short(state->current, state->remaining, &repeat_count)) {
    return GIF_READ_PAST_BUFFER;
  }

  uint8_t terminator_byte;
  if (!read_byte(state->current, state->remaining, &terminator_byte)) {
    return GIF_READ_PAST_BUFFER;
  }

  if (terminator_byte != 0) {
    return GIF_NETSCAPE_NULL_MISSING;
  }

  *state->remaining -= GIF_APPLICATION_EXTENSION_SIZE + 2U;
  state->details->repeat_count = repeat_count;
  return GIF_SUCCESS;
}

static gif_result_code read_extension_block(
    gif_parse_state* state,
    size_t frame_index,
    bool* seen_graphics_control_extension)
{
  uint8_t extension_type_byte;
  if (!read_byte(state->current, state->remaining, &extension_type_byte)) {
    return GIF_READ_PAST_BUFFER;
  }

  gif_extension_type extension_type = (gif_extension_type)extension_type_byte;
  switch (extension_type) {
    case GIF_GRAPHICS_CONTROL_EXTENSION: {
      if (*seen_graphics_control_extension) {
        return GIF_MULTIPLE_GRAPHICS_CONTROL_EXTENSIONS;
      }
      *seen_graphics_control_extension = true;

      TRY(read_graphics_control_extension(state, frame_index));
      break;
    }
    case GIF_APPLICATION_EXTENSION: {
      TRY(read_application_extension(state));
      break;
    }
    case GIF_COMMENT_EXTENSION:
      /* fallthrough */
    case GIF_TEXT_EXTENSION:
      if (!skip_block(state->current, state->remaining)) {
        return GIF_READ_PAST_BUFFER;
      }
      break;
    default:
      return GIF_UNKNOWN_EXTENSION;
  }

  return GIF_SUCCESS;
}

static bool is_frame_size_invalid(gif_frame_descriptor* descriptor)
{
  return (uint32_t)descriptor->width + (uint32_t)descriptor->left > 0xFFFFU
      || (uint32_t)descriptor->height + (uint32_t)descriptor->top > 0xFFFFU
      /* Although the GIF specification doesn't say anything about 0 sized
       * frames, those are rejected on the basis of not being useful */
      || descriptor->width == 0 || descriptor->height == 0;
}

static bool is_frame_out_of_bounds(gif_descriptor* descriptor,
                                   gif_frame_descriptor* frame_descriptor)
{
  return frame_descriptor->width + frame_descriptor->left
      > descriptor->canvas_width
      || frame_descriptor->height + frame_descriptor->top
      > descriptor->canvas_height;
}

#define GIF_IMAGE_DESCRIPTOR_SIZE 9U

static gif_result_code read_image_descriptor_block(gif_parse_state* state,
                                                   size_t frame_index)
{
  if (*state->remaining < GIF_IMAGE_DESCRIPTOR_SIZE) {
    return GIF_READ_PAST_BUFFER;
  }

  TRY(ensure_frame_data(state, frame_index));

  gif_frame_data* frame_data =
      &state->details->frame_vector.frames[frame_index];
  gif_frame_descriptor* descriptor = &frame_data->descriptor;
  descriptor->left = read_le_short_un(state->current);
  descriptor->top = read_le_short_un(state->current);
  descriptor->width = read_le_short_un(state->current);
  descriptor->height = read_le_short_un(state->current);

#define FRAME_CHECK(boolean, code) \
  do { \
    if (boolean) { \
      memcpy(&frame_index, &state->data, sizeof(size_t)); \
      return code; \
    } \
  } while (0)

  FRAME_CHECK(is_frame_size_invalid(descriptor), GIF_FRAME_SIZE_INVALID);
  FRAME_CHECK(is_frame_out_of_bounds(&state->details->descriptor, descriptor),
              GIF_FRAME_OUT_OF_BOUNDS);

  uint8_t packed_byte = read_byte_un(state->current);
  gif_frame_descriptor_packed* packed = &descriptor->packed;
  packed->local_color_table_flag = (packed_byte & B8(10000000)) != 0;
  packed->interlace_flag = (packed_byte & B8(01000000)) != 0;
  packed->sort_flag = (packed_byte & B8(00100000)) != 0;
  packed->size = packed_byte & B8(00000111);

  *state->remaining -= GIF_IMAGE_DESCRIPTOR_SIZE;

  if (packed->local_color_table_flag) {
    TRY(read_color_table(state->current,
                         state->remaining,
                         &frame_data->local_color_table,
                         packed->size,
                         state->allocator));
  }

  uint8_t min_code_size;
  if (!read_byte(state->current, state->remaining, &min_code_size)) {
    return GIF_READ_PAST_BUFFER;
  }

  uint8_t* first_subblock = *state->current + 1;
  size_t data_length = 0;
  while (1) {
    uint8_t subblock_size;
    if (!read_byte(state->current, state->remaining, &subblock_size)) {
      return GIF_READ_PAST_BUFFER;
    }
    if (subblock_size == 0) {
      break;
    }
    if (!skip_bytes(state->current, state->remaining, subblock_size)) {
      return GIF_READ_PAST_BUFFER;
    }
    data_length += subblock_size;
  }

  if (data_length == 0) {
    return GIF_FRAME_DATA_EMPTY;
  }

  frame_data->min_code_size = min_code_size;
  frame_data->first_subblock = first_subblock;
  frame_data->data_length = data_length;
  return GIF_SUCCESS;
}

typedef enum gif_block_type {
  GIF_EXTENSION_BLOCK = 0x21,
  GIF_IMAGE_DESCRIPTOR_BLOCK = 0x2C,
  GIF_TAIL_BLOCK = 0x3B,
} gif_block_type;

gif_result_code gif_parse_impl(gif_parse_state* state)
{
#define CONST_CHECK(predicate, fail_code) \
  do { \
    switch (predicate(state)) { \
      case CMP_EQ: \
        break; \
      case CMP_NEQ: \
        return fail_code; \
      case CMP_OOB: \
        return GIF_READ_PAST_BUFFER; \
    } \
  } while (0)

  CONST_CHECK(is_gif_file, GIF_NOT_A_GIF);
  CONST_CHECK(is_gif_version_supported, GIF_NOT_A_GIF89A);

  if (!read_descriptor(state)) {
    return GIF_READ_PAST_BUFFER;
  }

  if (state->details->descriptor.packed.global_color_table_flag) {
    TRY(read_color_table(state->current,
                         state->remaining,
                         &state->details->global_color_table,
                         state->details->descriptor.packed.size,
                         state->allocator));
  }

  size_t frame_index = 0;
  bool seen_graphics_control_extension = false;
  while (1) {
    uint8_t block_type_byte;
    if (!read_byte(state->current, state->remaining, &block_type_byte)) {
      return GIF_READ_PAST_BUFFER;
    }

    gif_block_type block_type = (gif_block_type)block_type_byte;
    switch (block_type) {
      case GIF_EXTENSION_BLOCK: {
        TRY(read_extension_block(
            state, frame_index, &seen_graphics_control_extension));
        break;
      }
      case GIF_IMAGE_DESCRIPTOR_BLOCK: {
        TRY(read_image_descriptor_block(state, frame_index));
        ++frame_index;
        seen_graphics_control_extension = false;
        break;
      }
      case GIF_TAIL_BLOCK:
        if (frame_index == 0) {
          return GIF_IMAGE_DESCRIPTOR_MISSING;
        }
        goto tail_block;
      default:
        return GIF_UNKNOWN_BLOCK;
    }
  }

tail_block:
  do {
    _Static_assert(sizeof(void*) >= sizeof(size_t),
                   "void* should have a size greater than or equal to size_t");
    memcpy(&state->data, state->remaining, sizeof(size_t));
  } while (0);

  return GIF_SUCCESS;
}
