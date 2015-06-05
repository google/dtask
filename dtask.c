#include <stdlib.h>
#include <stdio.h>

#include "dtask.h"

void dtask_enable(dtask_state_t *state, dtask_mask_t mask) {
  dtask_mask_t enabled = state->enabled | mask;
  dtask_id_t n = state->num_tasks;
  const dtask_t *t = state->tasks;

  for(unsigned int i = 0; i < n; i++, t++) {
    if((1U << i) & mask) {
      enabled |= t->dependencies;
    }
  }

  state->enabled = enabled;
}

void dtask_disable(dtask_state_t *state, dtask_mask_t mask) {
  dtask_mask_t enabled = state->enabled & ~mask;
  dtask_mask_t changed = mask & enabled;
  dtask_id_t n = state->num_tasks;
  const dtask_t *t = state->tasks;

  for(unsigned int i = 0; i < n; i++, t++) {
    if((1U << i) & mask) {
      enabled &= ~t->dependents;
    }
  }

  state->enabled = enabled;
}

void dtask_disable_all(dtask_state_t *state) {
  state->enabled = 0;
}

void dtask_run(dtask_state_t *state) {
  const dtask_t *t = state->tasks;
  dtask_id_t n = state->num_tasks;
  dtask_mask_t enabled = state->enabled;
  dtask_mask_t mask = 0;
  dtask_mask_t id_bit = 1;
  while(n--) {
    // task can depend on itself
    dtask_mask_t new_mask = mask | id_bit;
    if((id_bit & enabled) &&
       (t->direct_dependencies & new_mask) &&
       t->func(t, new_mask)) {
      mask = new_mask;
    }
    t++;
    id_bit <<= 1;
  }
}
