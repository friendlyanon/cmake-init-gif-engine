#include "gif_engine/gif_engine.h"

#include <string.h>

int main(int argc, const char* argv[])
{
  (void)argc;
  (void)argv;

  return strcmp("gif_engine", exported_function()) == 0 ? 0 : 1;
}
