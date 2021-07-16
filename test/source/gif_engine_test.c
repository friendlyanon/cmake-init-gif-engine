#include "gif_engine/gif_engine.h"
#include <utest.h>

UTEST(main, compare)
{
  ASSERT_STREQ("gif_engine", exported_function());
}

UTEST_MAIN()
