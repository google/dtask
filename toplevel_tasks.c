#include <stdio.h>
#include <assert.h>

#ifndef DTASK_GEN
#include "toplevel_tasks.h"
#include "factor_tasks.h"
#include "simple_tasks.h"
#endif

#define DEBUG 1

#if DEBUG
#define debugf(args...) printf(args)
#else
#define debugf(...)
#endif

DTASK_GROUP(toplevel_tasks)

extern dtask_config_t factor_config;
extern factor_tasks_state_t factor_state[2];
DTASK(factor_events, dtask_set_t)
{
  /* connect factor_tasks to simple_tasks */
  dtask_set_t initial = COUNT;
  int idx = *DREF(simple_toggle) ? 1 : 0; // being verbose to make indexing clear

  *DREF(factor_events) = dtask_run(&factor_config, &factor_state[idx], initial, events);
  return *DREF(factor_events) != 0;
}

extern dtask_config_t simple_config;
extern simple_tasks_state_t simple_state;
DTASK(simple_toggle, bool)
{
  dtask_set_t initial = TOGGLE;

  dtask_set_t simple_events = dtask_run(&simple_config, &simple_state, initial, events);

  // deoendency on simple_state.toggle cannot be handled automatically
  // because it is within another (sub)graph
  *DREF(simple_toggle) = simple_state.toggle;

  return (simple_events & TOGGLE) != 0;
}
