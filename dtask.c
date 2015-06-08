#include <stdlib.h>
#include <stdio.h>

#include "dtask.h"

void dtask_enable(dtask_state_t *state, dtask_mask_t mask) {
  dtask_mask_t enabled_dependencies = state->enabled |= mask;
  const dtask_id_t n = state->num_tasks;
  const dtask_t *t = state->tasks;

  // enable dependencies
  for(unsigned int i = 0; i < n; i++, t++) {
    if((1U << i) & mask) {
      enabled_dependencies |= t->all_dependencies;
    }
  }

  state->enabled_dependencies = enabled_dependencies;
}

void dtask_disable(dtask_state_t *state, dtask_mask_t mask) {
  dtask_mask_t enabled = state->enabled & ~mask;
  const dtask_id_t n = state->num_tasks;
  const dtask_t *t = state->tasks;

  // disable dependents
  for(unsigned int i = 0; i < n; i++, t++) {
    if((1U << i) & mask) {
      enabled &= ~t->all_dependents;
    }
  }

  state->enabled = enabled;

  // clear and re-enable dependencies
  state->enabled_dependencies = 0;
  dtask_enable(state, enabled);
}

void dtask_disable_all(dtask_state_t *state) {
  state->enabled = 0;
  state->enabled_dependencies = 0;
}

void dtask_run(dtask_state_t *state, dtask_mask_t initial) {
  const dtask_t *t = state->tasks;
  dtask_mask_t
    enabled = state->enabled_dependencies,
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
