#include "parse.h"

#include "buffer_ops.h"

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

static bool read_descriptor(uint8_t** begin,
                            uint8_t* end,
                            gif_descriptor* descriptor)
{
  if (end - *begin < LOGICAL_SCREEN_DESCRIPTOR_SIZE) {
    return false;
  }

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
  return 2ULL << size;
}

static gif_result_code read_global_color_table(uint8_t** begin,
                                               uint8_t* end,
                                               gif_details* details)
{
  size_t color_count = size_to_count(details->descriptor.packed.size);
  size_t color_bytes = color_count * 3;
  if ((size_t)(end - *begin) < color_bytes) {
    return GIF_READ_PAST_BUFFER;
  }

  uint32_t* buffer = allocator_(color_bytes * sizeof(uint32_t));
  if (buffer == NULL) {
    return GIF_ALLOC_FAIL;
  }

  for (size_t i = 0; i < color_count; ++i) {
    buffer[i] = read_color_un(begin);
  }

  details->global_color_table = buffer;
  return GIF_SUCCESS;
}

gif_result_code gif_parse_impl(void** data)
{
  uint8_t* current = buffer_;
  uint8_t* end = buffer_ + buffer_size_;

  switch (is_gif_file(&current, end)) {
    case CMP_EQ:
      break;
    case CMP_NEQ:
      return GIF_NOT_A_GIF;
    case CMP_OOB:
      *data = current;
      return GIF_READ_PAST_BUFFER;
  }

  switch (is_gif_version_supported(&current, end)) {
    case CMP_EQ:
      break;
    case CMP_NEQ:
      return GIF_NOT_A_GIF89A;
    case CMP_OOB:
      *data = current;
      return GIF_READ_PAST_BUFFER;
  }

  if (!read_descriptor(&current, end, &details_->descriptor)) {
    *data = current;
    return GIF_READ_PAST_BUFFER;
  }

  if (details_->descriptor.packed.global_color_table_flag) {
    gif_result_code code = read_global_color_table(&current, end, details_);
    if (code != GIF_SUCCESS) {
      *data = current;
      return code;
    }
  }

  // TODO

  return GIF_SUCCESS;
}
