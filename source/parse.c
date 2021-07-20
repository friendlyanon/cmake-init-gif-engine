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

static compare_result is_gif_file(uint8_t* begin, uint8_t* end)
{
  return is_eq_safe(begin, end, magic, sizeof(magic));
}

static uint8_t gif_version[] = {'8', '9', 'a'};

static compare_result is_gif_version_supported(uint8_t* begin, uint8_t* end)
{
  return is_eq_safe(begin, end, gif_version, sizeof(gif_version));
}

gif_result_code gif_parse_impl(void** data)
{
  uint8_t current = buffer_;
  uint8_t end = buffer_ + buffer_size_;

  switch (is_gif_file(current, end)) {
    case CMP_EQ:
      current += sizeof(magic);
      break;
    case CMP_NEQ:
      return GIF_NOT_A_GIF;
    case CMP_OOB:
      *data = current;
      return GIF_READ_PAST_BUFFER;
  }

  switch (is_gif_version_supported(current, end)) {
    case CMP_EQ:
      current += sizeof(gif_version);
      break;
    case CMP_NEQ:
      return GIF_NOT_A_GIF89A;
    case CMP_OOB:
      *data = current;
      return GIF_READ_PAST_BUFFER;
  }

  // TODO

  return GIF_SUCCESS;
}
