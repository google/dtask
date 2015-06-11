#include <stdlib.h>
#include <stdio.h>

#include "dtask.h"

#ifdef NO_CLZ
#define dtask_set_find_first(set, prev) ((prev) - 1)
#else
#define dtask_set_find_first(set, prev) (__builtin_clz(set))
#endif

static inline
dtask_set_t dtask_bit(dtask_id_t id) {
  return (1U << (sizeof(dtask_set_t) * 8 - 1)) >> id;
}

void dtask_enable(dtask_state_t *state, dtask_set_t set) {
  dtask_set_t enabled_dependencies = state->enabled |= set;
  const dtask_t *tasks = state->tasks;
  dtask_id_t n = state->num_tasks;

  // enable dependencies
  while(set) {
    n = dtask_set_find_first(set, n);
    enabled_dependencies |= tasks[n].all_dependencies;
    set &= ~dtask_bit(n);
  }

  state->enabled_dependencies = enabled_dependencies;
}

void dtask_disable(dtask_state_t *state, dtask_set_t set) {
  dtask_set_t enabled = state->enabled & ~set;
  const dtask_t *tasks = state->tasks;
  dtask_id_t n = state->num_tasks;

  // disable dependents
  while(set) {
    n = dtask_set_find_first(set, n);
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
