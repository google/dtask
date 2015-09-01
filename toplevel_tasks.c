#include <stdio.h>
#include <assert.h>

#ifndef DTASK_GEN
#include "factor_tasks.h"
#include "simple_tasks.h"
#include "toplevel_tasks.h"
#endif

#define DEBUG 1

#if DEBUG
#define debugf(args...) printf(args)
#else
#define debugf(...)
#endif

DTASK_GROUP(toplevel_tasks)

extern dtask_state_t factor;
DTASK(factor_events, dtask_set_t)
{
  /* connect factor_tasks to simple_tasks */
  dtask_set_t initial = 0;

  /* TOGGLE triggers COUNT */
  if(*DREF(simple_events) & TOGGLE2) {
    initial |= COUNT;
  }

  factor_events = dtask_run(&factor, initial, events);
  return factor_events != 0;
}

extern dtask_state_t simple;
DTASK(simple_events, dtask_set_t)
{
  dtask_set_t initial = TOGGLE;

  simple_events = dtask_run(&simple, initial, events);
  return simple_events != 0;
}
