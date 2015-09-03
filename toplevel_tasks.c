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
extern factor_tasks_state_t factor_state;
DTASK(factor_events, dtask_set_t)
{
  /* connect factor_tasks to simple_tasks */
  dtask_set_t initial = 0;

  /* TOGGLE triggers COUNT */
  if(*DREF(simple_events) & TOGGLE2) {
    initial |= COUNT;
  }

  *DREF(factor_events) = dtask_run(&factor_config, &factor_state, initial, events);
  return *DREF(factor_events) != 0;
}

extern dtask_config_t simple_config;
extern simple_tasks_state_t simple_state;
DTASK(simple_events, dtask_set_t)
{
  dtask_set_t initial = TOGGLE;

  *DREF(simple_events) = dtask_run(&simple_config, &simple_state, initial, events);

  /* TOGGLE2 is the only 'exported' event */
  return (*DREF(simple_events) & TOGGLE2) != 0;
}
