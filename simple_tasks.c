#include <stdio.h>
#include <assert.h>

#ifndef DTASK_GEN
#include "simple_tasks.h"
#endif

#define DEBUG 1

#if DEBUG
#define debugf(args...) printf(args)
#else
#define debugf(...)
#endif

DTASK_GROUP(simple_tasks)

DTASK(toggle, bool) {
  *DREF(toggle) = !*DREF(toggle);
  return true;
}
