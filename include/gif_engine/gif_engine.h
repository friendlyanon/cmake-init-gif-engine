#pragma once

#include <gif_engine/error.h>
#include <gif_engine/export.h>
#include <gif_engine/structs.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*gif_allocator)(void* pointer, size_t size);
typedef void (*gif_deallocator)(void* allocation);

GIF_ENGINE_EXPORT gif_result gif_parse(uint8_t* buffer,
                                       size_t buffer_size,
                                       gif_details* details,
                                       gif_allocator allocator);

GIF_ENGINE_EXPORT void* gif_pop_last_position(void);

GIF_ENGINE_EXPORT gif_result gif_decode(gif_details* details,
                                        gif_allocator allocator);

GIF_ENGINE_EXPORT gif_result gif_free_details(gif_details* details,
                                              gif_deallocator deallocator);

#ifdef __cplusplus
}  // extern "C"
#endif
