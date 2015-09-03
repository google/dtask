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

extern dtask_state_t factor;
extern factor_tasks_data_t factor_data;
DTASK(factor_events, dtask_set_t)
{
  /* connect factor_tasks to simple_tasks */
  dtask_set_t initial = 0;

  /* TOGGLE triggers COUNT */
  if(*DREF(simple_events) & TOGGLE2) {
    initial |= COUNT;
  }

  *DREF(factor_events) = dtask_run(&factor, &factor_data, initial, events);
  return *DREF(factor_events) != 0;
}

extern dtask_state_t simple;
extern simple_tasks_data_t simple_data;
DTASK(simple_events, dtask_set_t)
{
  dtask_set_t initial = TOGGLE;

  *DREF(simple_events) = dtask_run(&simple, &simple_data, initial, events);

  /* TOGGLE2 is the only 'exported' event */
  return (*DREF(simple_events) & TOGGLE2) != 0;
}
