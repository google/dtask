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

static void dtask_update_enabled_dependencies(dtask_state_t *state, dtask_set_t set) {
  dtask_select_t *select = &state->select;
  select->enabled |= set;
  dtask_set_t enabled_dependencies = select->enabled_dependencies | set;
  const dtask_t *tasks = state->config.tasks;
  dtask_id_t n = 0;

  // enable dependencies
  while(set) {
    n = dtask_set_find_first(set, n);
    enabled_dependencies |= tasks[n].all_dependencies;
    set &= ~dtask_bit(n);
  }

  select->enabled_dependencies = enabled_dependencies;
}

static void dtask_update_disabled_dependents(dtask_state_t *state, dtask_set_t set) {
  dtask_select_t *select = &state->select;
  select->disabled |= set;
  dtask_set_t disabled_dependents = select->disabled_dependents | set;
  const dtask_t *tasks = state->config.tasks;
  dtask_id_t n = 0;

  // disable dependents
  while(set) {
    n = dtask_set_find_first(set, n);
    disabled_dependents |= tasks[n].all_dependents;
    set &= ~dtask_bit(n);
  }

  select->disabled_dependents = disabled_dependents;
}

static void dtask_call_enable_functions(dtask_state_t *state) {
  dtask_select_t *select = &state->select;
  dtask_set_t new_enabled = select->requested & ~select->selected;
  const dtask_t *tasks = state->config.tasks;
  dtask_id_t n = 0;

  // call enable functions
  while(new_enabled) {
    n = dtask_set_find_first(new_enabled, n);
    tasks[n].enable_func(state);
    new_enabled &= ~dtask_bit(n);
  }

  select->selected |= select->requested;
}

static void dtask_call_disable_functions(dtask_state_t *state) {
  dtask_select_t *select = &state->select;
  dtask_set_t new_disabled = ~select->requested & select->selected;
  const dtask_t *tasks = state->config.tasks;
  dtask_id_t n = DTASK_MAX_ID;

  // call disable functions (in reverse order, before dependencies)
  while(new_disabled) {
    n = dtask_set_find_last(new_disabled, n);
    tasks[n].disable_func(state);
    new_disabled &= ~dtask_bit(n);
  }

  select->selected &= select->requested;
}

void dtask_enable(dtask_state_t *state, dtask_set_t set)
{
  dtask_select_t *select = &state->select;
  dtask_update_enabled_dependencies(state, set);

  // clear conflicting disables and recompute
  if(select->disabled & select->enabled_dependencies) {
    select->disabled &= ~select->enabled_dependencies;
    select->disabled_dependents = 0;
    dtask_update_disabled_dependents(state, select->disabled);
  }

  // update requested
  select->requested = select->enabled_dependencies & ~select->disabled_dependents;
}

void dtask_disable(dtask_state_t *state, dtask_set_t set)
{
  dtask_select_t *select = &state->select;
  dtask_update_disabled_dependents(state, set);

  // clear directly conflicting enables and recompute
  if(select->enabled & set) {
    select->enabled &= ~set;
    select->enabled_dependencies = 0;
    dtask_update_enabled_dependencies(state, select->enabled);
  }

  // update requested
  select->requested = select->enabled_dependencies & ~select->disabled_dependents;
}

void dtask_clear(dtask_state_t *state, dtask_set_t set) {
  dtask_select_t *select = &state->select;

  // clear directly conflicting enables and recompute
  if(select->enabled & set) {
    select->enabled &= ~set;
    select->enabled_dependencies = 0;
    dtask_update_enabled_dependencies(state, select->enabled);
  }

  // clear directly conflicting disables and recompute
  if(select->disabled & set) {
    select->disabled &= ~set;
    select->disabled_dependents = 0;
    dtask_update_disabled_dependents(state, select->disabled);
  }

  // update requested
  select->requested = select->enabled_dependencies & ~select->disabled_dependents;
}

void dtask_switch(dtask_state_t *state, dtask_set_t set) {
  dtask_select_t *select = &state->select;

  // reset select
  select->enabled = 0;
  select->disabled = 0;
  select->enabled_dependencies = 0;
  select->disabled_dependents = 0;

  // enable set
  dtask_update_enabled_dependencies(state, set);
  select->requested = select->enabled_dependencies;
}

// call enable/disable functions
void dtask_select(dtask_state_t *state) {
  dtask_select_t *select = &state->select;
  if(select->requested != select->selected) {
    dtask_call_disable_functions(state);
    dtask_call_enable_functions(state);
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
