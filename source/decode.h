#pragma once

#include "gif_engine/gif_engine.h"

gif_result_code gif_decode_impl(void** data,
                                gif_details* details,
                                gif_allocator allocator);
