#include <stdlib.h>
#include <stdio.h>

#include "dtask.h"

static inline
dtask_id_t dtask_set_find_first(dtask_set_t set) {
  return __builtin_clz(set);
}

static inline
dtask_set_t dtask_bit(dtask_id_t id) {
  return (1U << (sizeof(dtask_set_t) * 8 - 1)) >> id;
}

void dtask_enable(dtask_state_t *state, dtask_set_t set) {
  dtask_set_t enabled_dependencies = state->enabled |= set;
  const dtask_t *tasks = state->tasks;

  // enable dependencies
  while(set) {
    dtask_id_t n = dtask_set_find_first(set);
    enabled_dependencies |= tasks[n].all_dependencies;
    set &= ~dtask_bit(n);
  }

  state->enabled_dependencies = enabled_dependencies;
}

void dtask_disable(dtask_state_t *state, dtask_set_t set) {
  dtask_set_t enabled = state->enabled & ~set;
  const dtask_t *tasks = state->tasks;

  // disable dependents
  while(set) {
    dtask_id_t n = dtask_set_find_first(set);
    enabled &= ~tasks[n].all_dependents;
    set &= ~dtask_bit(n);
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

void dtask_run(const dtask_state_t *state, dtask_set_t initial) {
  const dtask_set_t enabled = state->enabled_dependencies;
  dtask_set_t
    scheduled = initial & enabled,
    events = 0;

  while(scheduled) {
    const dtask_id_t active = dtask_set_find_first(scheduled);
    const dtask_set_t id_bit = dtask_bit(active);
    const dtask_t *task = &state->tasks[active];
    if((id_bit & scheduled) &&
       task->func(task, events)) {
      events |= id_bit;
      scheduled |= task->dependents & enabled;
    }
    scheduled &= ~id_bit;
  }
}
