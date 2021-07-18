#include "gif_engine/gif_engine.h"

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

UTEST(main, compare)
{
  ASSERT_STREQ("gif_engine", exported_function());
}

UTEST_MAIN()
