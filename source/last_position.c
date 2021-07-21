#include "last_position.h"

#include <assert.h>
#include <stddef.h>

#include "gif_engine/gif_engine.h"

static void* last_position_ = NULL;

void gif_set_last_position(void* position)
{
  assert(position == NULL || last_position_ == NULL);
  last_position_ = position;
}

void* gif_pop_last_position(void)
{
  void* position = last_position_;
  last_position_ = NULL;
  return position;
}
