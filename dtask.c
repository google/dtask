#include <stdlib.h>
#include <stdio.h>

#include "dtask.h"

void dtask_run(dtask_t *tasks, size_t num) {
  for(;;) {
    dtask_t *t = tasks;
    size_t n = num;
    uint32_t mask = 0;
    while(n--) {
      // task can depend on itself
      uint32_t new_mask = mask | (1U << t->id);
      if((t->mask & new_mask) &&
         t->func(t, new_mask)) {
        mask = new_mask;
      }
      t++;
    }
  }
}
