#ifndef __DTASK__
#define __DTASK__

#include <stdint.h>
#include <stdbool.h>
#include "all_tasks.h"

struct dtask;
typedef struct dtask dtask_t;
struct dtask
{
  bool (*func)(dtask_t *, uint32_t);
  char *name;
  uint32_t mask;
  uint8_t id;
};

#define DGET(x) (x)

#define DTASK(name, type)  \
  type name;               \
  bool dtask_##name(dtask_t *task, uint32_t mask)

#endif
