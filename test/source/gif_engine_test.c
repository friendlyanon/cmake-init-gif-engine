#include <gif_engine/gif_engine.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <utest.h>

#include "gif_mmap.h"

struct parser_fixture_2frame {
  gif_mmap_span span;
};

UTEST_F_SETUP(parser_fixture_2frame)
{
  /* Arrange */
  const char* file = "2frame.gif";

  /* Act */
  gif_mmap_span span = gif_mmap_allocate(file);
  if (span.pointer == NULL) {
    gif_mmap_print_last_error_to_stderr();
  }

  utest_fixture->span = span;

  /* Assert */
  ASSERT_NE(span.pointer, NULL);
  ASSERT_EQ(span.size, 199U);
}

UTEST_F_TEARDOWN(parser_fixture_2frame)
{
  /* Arrange */

  /* Act */
  bool cleanup_was_successful = gif_mmap_deallocate(&utest_fixture->span);

  /* Assert */
  ASSERT_TRUE(cleanup_was_successful);
}

#define RED 0x00FF0000U
#define BLUE 0x000000FFU
#define BLACK 0x00000000U
#define WHITE 0x00FFFFFFU

// NOLINTNEXTLINE(readability-function-size)
UTEST_F(parser_fixture_2frame, parse)
{
  /* Arrange */
  gif_details details;

  /* Act */
  gif_result parse_result = gif_parse(utest_fixture->span.pointer,
                                      utest_fixture->span.size,
                                      &details,
                                      &realloc);
  size_t leftover_bytes;
  memcpy(&leftover_bytes, &parse_result.data, sizeof(size_t));

  /* Assert */
  ASSERT_EQ((int)parse_result.code, GIF_SUCCESS);
  ASSERT_EQ(leftover_bytes, 1U);

  ASSERT_EQ(details.descriptor.canvas_width, 2);
  ASSERT_EQ(details.descriptor.canvas_height, 1);

  ASSERT_EQ(details.descriptor.packed.global_color_table_flag, true);
  ASSERT_EQ(details.descriptor.packed.color_resolution, 1);
  ASSERT_EQ(details.descriptor.packed.sort_flag, false);
  ASSERT_EQ(details.descriptor.packed.size, 0);

  ASSERT_EQ(details.descriptor.background_color_index, 0);
  ASSERT_EQ(details.descriptor.pixel_aspect_ratio, 0);

  uint32_t* global_color_table = details.global_color_table;
  ASSERT_NE(global_color_table, NULL);
  ASSERT_EQ(global_color_table[0], RED);
  ASSERT_EQ(global_color_table[1], BLUE);

  ASSERT_EQ(details.repeat_count, 1);

  gif_frame_vector frame_vector = details.frame_vector;
  ASSERT_NE(frame_vector.frames, NULL);
  ASSERT_EQ(frame_vector.size, 2U);

  gif_frame_data frame1 = frame_vector.frames[0];
  gif_graphic_extension extension1 = frame1.graphic_extension;
  ASSERT_EQ((int)extension1.packed.disposal_method, GIF_DISPOSAL_NOTHING);
  ASSERT_EQ(extension1.packed.user_input_flag, false);
  ASSERT_EQ(extension1.packed.transparent_color_flag, true);

  ASSERT_EQ(extension1.delay, 6);
  ASSERT_EQ(extension1.transparent_color_index, 0);

  gif_frame_descriptor descriptor1 = frame1.descriptor;
  ASSERT_EQ(descriptor1.left, 0);
  ASSERT_EQ(descriptor1.top, 0);
  ASSERT_EQ(descriptor1.width, 2);
  ASSERT_EQ(descriptor1.height, 1);

  ASSERT_EQ(descriptor1.packed.local_color_table_flag, false);
  ASSERT_EQ(descriptor1.packed.interlace_flag, false);
  ASSERT_EQ(descriptor1.packed.sort_flag, false);
  ASSERT_EQ(descriptor1.packed.size, 0);

  ASSERT_EQ(frame1.local_color_table, NULL);
  ASSERT_EQ(frame1.min_code_size, 0x88);
  ASSERT_EQ(frame1.data_length, 1U);

  gif_frame_data frame2 = frame_vector.frames[1];
  gif_graphic_extension extension2 = frame2.graphic_extension;
  ASSERT_EQ((int)extension2.packed.disposal_method, GIF_DISPOSAL_BACKGROUND);
  ASSERT_EQ(extension2.packed.user_input_flag, true);
  ASSERT_EQ(extension2.packed.transparent_color_flag, false);

  ASSERT_EQ(extension2.delay, 0x60);
  ASSERT_EQ(extension2.transparent_color_index, 1);

  gif_frame_descriptor descriptor2 = frame2.descriptor;
  ASSERT_EQ(descriptor2.left, 0);
  ASSERT_EQ(descriptor2.top, 0);
  ASSERT_EQ(descriptor2.width, 2);
  ASSERT_EQ(descriptor2.height, 1);

  ASSERT_EQ(descriptor2.packed.local_color_table_flag, true);
  ASSERT_EQ(descriptor2.packed.interlace_flag, true);
  ASSERT_EQ(descriptor2.packed.sort_flag, true);
  ASSERT_EQ(descriptor2.packed.size, 1);

  uint32_t* local_color_table = frame2.local_color_table;
  ASSERT_NE(frame2.local_color_table, NULL);
  ASSERT_EQ(local_color_table[0], BLUE);
  ASSERT_EQ(local_color_table[1], WHITE);
  ASSERT_EQ(local_color_table[2], RED);
  ASSERT_EQ(local_color_table[3], BLACK);

  ASSERT_EQ(frame2.min_code_size, 0x11);
  ASSERT_EQ(frame2.data_length, 2U);

  /* Cleanup */
  gif_free_details(&details, &free);
}

struct parser_fixture_11frame {
  gif_mmap_span span;
};

UTEST_F_SETUP(parser_fixture_11frame)
{
  /* Arrange */
  const char* file = "11frame.gif";

  /* Act */
  gif_mmap_span span = gif_mmap_allocate(file);
  if (span.pointer == NULL) {
    gif_mmap_print_last_error_to_stderr();
  }

  utest_fixture->span = span;

  /* Assert */
  ASSERT_NE(span.pointer, NULL);
  ASSERT_EQ(span.size, 238U);
}

UTEST_F_TEARDOWN(parser_fixture_11frame)
{
  /* Arrange */

  /* Act */
  bool cleanup_was_successful = gif_mmap_deallocate(&utest_fixture->span);

  /* Assert */
  ASSERT_TRUE(cleanup_was_successful);
}

static void* fake_realloc(void* allocation, size_t size)
{
  /* Trigger the realloc failure path when the frame data vector resizes */
  return allocation == NULL ? malloc(size) : NULL;
}

UTEST_F(parser_fixture_11frame, realloc_fail)
{
  /* Arrange */
  gif_details details;

  /* Act */
  gif_result parse_result = gif_parse(utest_fixture->span.pointer,
                                      utest_fixture->span.size,
                                      &details,
                                      &fake_realloc);
  /* We can free early here, because this test only checks the return code */
  gif_free_details(&details, &free);

  /* Assert */
  ASSERT_EQ((int)parse_result.code, GIF_REALLOC_FAIL);
}

UTEST_MAIN()
