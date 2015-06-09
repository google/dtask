#include <stdlib.h>
#include <stdio.h>

#include "dtask.h"

static inline
dtask_id_t dtask_set_find_first(dtask_set_t set) {
  return sizeof(set) * 8 - (__builtin_clz(set) + 1);
}

void dtask_enable(dtask_state_t *state, dtask_set_t set) {
  dtask_set_t enabled_dependencies = state->enabled |= set;
  const dtask_t *tasks = state->tasks;
  const dtask_id_t last = state->num_tasks - 1;

  // enable dependencies
  while(set) {
    dtask_id_t n = dtask_set_find_first(set);
    enabled_dependencies |= tasks[last - n].all_dependencies;
    set &= ~(1U << n);
  }

  state->enabled_dependencies = enabled_dependencies;
}

void dtask_disable(dtask_state_t *state, dtask_set_t set) {
  dtask_set_t enabled = state->enabled & ~set;
  const dtask_t *tasks = state->tasks;
  const dtask_id_t last = state->num_tasks - 1;

  // disable dependents
  while(set) {
    dtask_id_t n = dtask_set_find_first(set);
    enabled &= ~tasks[last - n].all_dependents;
    set &= ~(1U << n);
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
  dtask_id_t last = state->num_tasks - 1;

  while(scheduled) {
    const dtask_id_t active = dtask_set_find_first(scheduled);
    const dtask_set_t id_bit = 1U << active;
    const dtask_t *task = &state->tasks[last - active];
    if((id_bit & scheduled) &&
       task->func(task, events)) {
      events |= id_bit;
      scheduled |= task->dependents & enabled;
    }
    scheduled &= ~id_bit;
  }
}
