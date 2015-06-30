#include <stdlib.h>
#include <stdio.h>

#include "dtask.h"

void __dtask_noop() {}

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

#ifdef NO_CTZ
dtask_id_t dtask_set_find_last(dtask_set_t set, dtask_id_t prev) {
  dtask_id_t x = DTASK_MAX_ID - prev;
  dtask_set_t s = set >> x;
  while(s) {
    if(1 & set) {
      return DTASK_MAX_ID - x;
    }
    x++;
    s >>= 1;
  };
  return -1;
}
#else
#define dtask_set_find_last(set, prev) (DTASK_MAX_ID - __builtin_ctz(set))
#endif

static void dtask_state_enable(dtask_state_t *state, dtask_set_t set) {
  state->enabled |= set;
  dtask_set_t enabled_dependencies = state->enabled_dependencies | set;
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

static void dtask_call_enable_functions(dtask_state_t *state, dtask_set_t prev_enabled) {
  dtask_set_t new_enabled = state->enabled_dependencies & ~prev_enabled;
  const dtask_t *tasks = state->tasks;
  dtask_id_t n = 0;

  // call enable functions
  while(new_enabled) {
    n = dtask_set_find_first(new_enabled, n);
    tasks[n].enable_func();
    new_enabled &= ~dtask_bit(n);
  }
}

void dtask_enable(dtask_state_t *state, dtask_set_t set)
{
  dtask_set_t prev_enabled = state->enabled_dependencies;
  dtask_state_enable(state, set);
  dtask_call_enable_functions(state, prev_enabled);
}

static void dtask_state_disable(dtask_state_t *state, dtask_set_t set) {
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
  dtask_state_enable(state, enabled);
}

static void dtask_call_disable_functions(dtask_state_t *state, dtask_set_t prev_enabled) {
  dtask_set_t new_disabled = ~state->enabled_dependencies & prev_enabled;
  const dtask_t *tasks = state->tasks;
  dtask_id_t n = DTASK_MAX_ID;

  // call disable functions (in reverse order, before dependencies)
  while(new_disabled) {
    n = dtask_set_find_last(new_disabled, n);
    tasks[n].disable_func();
    new_disabled &= ~dtask_bit(n);
  }
}

void dtask_disable(dtask_state_t *state, dtask_set_t set)
{
  dtask_set_t prev_enabled = state->enabled_dependencies;
  dtask_state_disable(state, set);
  dtask_call_disable_functions(state, prev_enabled);
}

void dtask_switch(dtask_state_t *state, dtask_set_t set) {
  dtask_set_t prev_enabled = state->enabled_dependencies;
  state->enabled = 0;
  state->enabled_dependencies = 0;
  dtask_state_enable(state, set);
  dtask_call_disable_functions(state, prev_enabled);
  dtask_call_enable_functions(state, prev_enabled);
}

#ifndef NO_CLZ
dtask_set_t dtask_run(const dtask_state_t *state, dtask_set_t initial) {
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
  return events;
}
#else
dtask_set_t dtask_run(const dtask_state_t *state, dtask_set_t initial) {
  return state->run(state, initial);
}
#endif
