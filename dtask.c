#include <stdlib.h>
#include <stdio.h>

#include "dtask.h"

void __dtask_noop(dtask_state_t *state) {}

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

#define set_foreach(set, action)                \
  do {                                          \
    dtask_id_t n = 0;                           \
    dtask_set_t left = (set);                   \
    while(left) {                               \
      n = dtask_set_find_first(left, n);        \
      action;                                   \
      left &= ~dtask_bit(n);                    \
    }                                           \
  } while(0)

#define set_foreach_rev(set, action)            \
  do {                                          \
    dtask_id_t n = DTASK_MAX_ID;                \
    dtask_set_t left = (set);                   \
    while(left) {                               \
      n = dtask_set_find_last(left, n);         \
      action;                                   \
      left &= ~dtask_bit(n);                    \
    }                                           \
  } while(0)

static dtask_set_t dtask_requested(dtask_state_t *state) {
  dtask_select_t *select = &state->select;
  dtask_set_t requested = select->enabled;
  const dtask_t *tasks = state->config.tasks;

  // remove disabled dependents
  set_foreach(select->disabled,
              requested &= ~tasks[n].all_dependents);

  // add dependencies
  set_foreach(requested,
              requested |= tasks[n].all_dependencies);

  return requested;
}

static void dtask_call_enable_functions(dtask_state_t *state, dtask_set_t set) {
  const dtask_t *tasks = state->config.tasks;
  set_foreach(set,
              tasks[n].enable_func(state));
}

static void dtask_call_disable_functions(dtask_state_t *state, dtask_set_t set) {
  const dtask_t *tasks = state->config.tasks;
  set_foreach_rev(set,
                  tasks[n].disable_func(state));
}

void dtask_enable(dtask_state_t *state, dtask_set_t set)
{
  dtask_select_t *select = &state->select;
  const dtask_t *tasks = state->config.tasks;

  if(set & ~select->enabled) {
    select->dirty = true;
    select->enabled |= set;

    // clear conflicting disables
    select->disabled &= ~set;
    set_foreach(set,
                select->disabled &= ~tasks[n].all_dependencies);
  }
}

void dtask_disable(dtask_state_t *state, dtask_set_t set)
{
  dtask_select_t *select = &state->select;
  if(set & ~select->disabled) {
    select->dirty = true;
    select->disabled |= set;
    select->enabled &= ~set;
  }
}

void dtask_clear(dtask_state_t *state, dtask_set_t set) {
  dtask_select_t *select = &state->select;

  // clear directly conflicting enables and recompute
  if((select->enabled | select->disabled) & set) {
    select->dirty = true;
    select->enabled &= ~set;
    select->disabled &= ~set;
  }
}

void dtask_switch(dtask_state_t *state, dtask_set_t set) {
  dtask_clear(state, ~(dtask_set_t)0);
  dtask_enable(state, set);
}

void dtask_select(dtask_state_t *state) {
  dtask_select_t *select = &state->select;
  if(select->dirty) {
    dtask_set_t requested = dtask_requested(state);
    dtask_call_disable_functions(state, ~requested & select->selected);
    dtask_call_enable_functions(state, requested & ~select->selected);
    select->selected = requested;
    select->dirty = false;
  }
}

#ifndef NO_CLZ
dtask_set_t dtask_run(dtask_state_t *state, dtask_set_t initial) {
  dtask_select(state);

  const dtask_set_t selected = state->select.selected;
  dtask_set_t
    scheduled = initial & selected,
    active = 0;

  state->events = 0;

  while(scheduled) {
    active = dtask_set_find_first(scheduled, active);
    const dtask_set_t id_bit = dtask_bit(active);
    const dtask_t *task = &state->config.tasks[active];
    if((id_bit & scheduled) &&
       task->func(state)) {
      state->events |= id_bit;
      scheduled |= task->dependents & selected;
    }
    scheduled &= ~id_bit;
  }
  return state->events;
}
#else
dtask_set_t dtask_run(dtask_state_t *state, dtask_set_t initial) {
  dtask_select(state);
  return state->config.run(state, initial);
}
#endif
