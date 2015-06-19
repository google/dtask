#include <stdlib.h>
#include <stdio.h>

#include "dtask.h"

#ifdef NO_CLZ
dtask_id_t dtask_set_find_first(dtask_set_t set, dtask_id_t prev) {
  dtask_id_t x = prev;
  dtask_set_t s = set << x;
  while(s) {
    if(dtask_bit(0) & s) {
      return x;
    }
    x++;
    s <<= 1;
  }
  return DTASK_BIT_WIDTH(dtask_set_t);
}
#else
#define dtask_set_find_first(set, prev) (__builtin_clz(set))
#endif

void dtask_enable(dtask_state_t *state, dtask_set_t set) {
  dtask_set_t enabled_dependencies = state->enabled |= set;
  const dtask_t *tasks = state->tasks;
  dtask_id_t n = 0;

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
  dtask_id_t n = 0;

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

#ifndef NO_CLZ
void dtask_run(const dtask_state_t *state, dtask_set_t initial) {
  const dtask_set_t enabled = state->enabled_dependencies;
  dtask_set_t
    scheduled = initial & enabled,
    events = 0,
    active = 0;

  while(scheduled) {
    active = dtask_set_find_first(scheduled, active);
    const dtask_set_t id_bit = dtask_bit(active);
    const dtask_t *task = &state->tasks[active];
    if((id_bit & scheduled) &&
       task->func(events)) {
      events |= id_bit;
      scheduled |= task->dependents & enabled;
    }
    scheduled &= ~id_bit;
  }
}
#else
void dtask_run(const dtask_state_t *state, dtask_set_t initial) {
  state->run(state, initial);
}
#endif
