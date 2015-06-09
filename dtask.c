#include <stdlib.h>
#include <stdio.h>

#include "dtask.h"

void dtask_enable(dtask_state_t *state, dtask_set_t set) {
  dtask_set_t enabled_dependencies = state->enabled |= set;
  dtask_id_t n = state->num_tasks;
  const dtask_t *t = state->tasks;

  // enable dependencies
  while(n--) {
    if((1U << n) & set) {
      enabled_dependencies |= t->all_dependencies;
    }
    t++;
  }

  state->enabled_dependencies = enabled_dependencies;
}

void dtask_disable(dtask_state_t *state, dtask_set_t set) {
  dtask_set_t enabled = state->enabled & ~set;
  dtask_id_t n = state->num_tasks;
  const dtask_t *t = state->tasks;

  // disable dependents
  while(n--) {
    if((1U << n) & set) {
      enabled &= ~t->all_dependents;
    }
    t++;
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

static inline
dtask_id_t dtask_set_find_first(dtask_set_t set) {
  return sizeof(set) * 8 - (__builtin_clz(set) + 1);
}

void dtask_run(dtask_state_t *state, dtask_set_t initial) {
  const dtask_t *t = state->tasks;
  dtask_set_t
    enabled = state->enabled_dependencies,
    scheduled = initial & enabled,
    events = 0,
    id_bit = 1U << dtask_set_find_first(initial);

  while(scheduled) {
    if((id_bit & scheduled) &&
       t->func(t, events)) {
      events |= id_bit;
      scheduled |= t->dependents & enabled;
    }
    scheduled &= ~id_bit;
    id_bit >>= 1;
    t++;
  }
}
