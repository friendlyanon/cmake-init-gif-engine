#include <gif_engine/gif_engine.h>
#include <stdbool.h>
#include <stddef.h>
#include <utest.h>

#include "gif_mmap.h"

UTEST(mmap, main)
{
  gif_mmap_span span = gif_mmap_allocate(NULL);
  ASSERT_EQ(span.size, 0);
  if (span.pointer == NULL) {
    gif_mmap_print_last_error_to_stderr();
  } else {
    gif_mmap_deallocate(span.pointer);
  }
}

UTEST(main, api)
{
  gif_result parse_result = gif_parse(NULL, 0, NULL, NULL);
  gif_result decode_result = gif_decode(NULL, NULL);
  gif_result free_result = gif_free_details(NULL, NULL);

  ASSERT_EQ(parse_result.code, GIF_SUCCESS);
  ASSERT_EQ(decode_result.code, GIF_SUCCESS);
  ASSERT_EQ(free_result.code, GIF_SUCCESS);
}

UTEST_MAIN()
