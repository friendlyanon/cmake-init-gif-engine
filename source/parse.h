#pragma once

#include <stddef.h>
#include <stdint.h>

#include "gif_engine/gif_engine.h"

void gif_parse_detail_set_globals(uint8_t* buffer,
                                  size_t buffer_size,
                                  gif_details* details,
                                  gif_allocator allocator);

gif_result gif_parse_impl(void** data);
