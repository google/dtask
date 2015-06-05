#include <stdlib.h>
#include <stdio.h>

#include "dtask.h"

void dtask_enable(dtask_state_t *state, dtask_mask_t mask) {
  dtask_mask_t enabled = state->enabled | mask;
  const dtask_id_t n = state->num_tasks;
  const dtask_t *t = state->tasks;

  for(unsigned int i = 0; i < n; i++, t++) {
    if((1U << i) & mask) {
      enabled |= t->all_dependencies;
    }
  }

  state->enabled = enabled;
}

void dtask_disable(dtask_state_t *state, dtask_mask_t mask) {
  dtask_mask_t enabled = state->enabled & ~mask;
  const dtask_id_t n = state->num_tasks;
  const dtask_t *t = state->tasks;

  for(unsigned int i = 0; i < n; i++, t++) {
    if((1U << i) & mask) {
      enabled &= ~t->all_dependents;
    }
  }

  state->enabled = enabled;
}

void dtask_disable_all(dtask_state_t *state) {
  state->enabled = 0;
}

void dtask_run(dtask_state_t *state, dtask_mask_t initial) {
  const dtask_t *t = state->tasks;
  dtask_mask_t
    enabled = state->enabled,
    scheduled = initial & enabled,
    events = 0,
    id_bit = 1;

  while(scheduled) {
    if((id_bit & scheduled) &&
       t->func(t, events)) {
      events |= id_bit;
      scheduled |= t->dependents & enabled;
    }
    scheduled &= ~id_bit;
    id_bit <<= 1;
    t++;
  }
}
