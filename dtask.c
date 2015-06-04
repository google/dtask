#include <stdlib.h>
#include <stdio.h>

#include "dtask.h"

void dtask_enable(dtask_state_t *state, dtask_mask_t mask) {
  dtask_mask_t enabled = state->enabled;
  dtask_mask_t changed = mask & ~enabled;
  dtask_id_t n = state->num_tasks;

  while(changed) {
    enabled |= changed; // enable the changed bits
    const dtask_t *t = state->tasks;
    for(unsigned int i = 0; i < n; i++, t++) {
      if((1U << i) & enabled) { // if task i is enabled
        changed |= t->depends; // change the dependencies
      }
    }
    changed &= ~enabled; // diff changed and enabled
  }
  state->enabled = enabled;
}

void dtask_disable(dtask_state_t *state, dtask_mask_t mask) {
  dtask_mask_t enabled = state->enabled;
  dtask_mask_t changed = mask & enabled;
  dtask_id_t n = state->num_tasks;

  while(changed) {
    enabled &= ~changed; // disable the changed bits
    const dtask_t *t = state->tasks;
    for(unsigned int i = 0; i < n; i++, t++) {
      if(t->depends & ~enabled) { // if any dependencies are disabled
        changed |= (1U << i); // change task i
      }
    }
    changed &= enabled; // diff changed and disabled (~enabled)
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
       (t->depends & new_mask) &&
       t->func(t, new_mask)) {
      mask = new_mask;
    }
    t++;
    id_bit <<= 1;
  }
}
