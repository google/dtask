/* Copyright 2016 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#include <stdio.h>
#include <assert.h>
#include "types.h"

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

extern factor_tasks_state_t factor_state[2];
extern simple_tasks_state_t simple_state;

DTASK(factor_events, dtask_set_t)
{
  /* connect factor_tasks to simple_tasks */
  dtask_set_t initial = COUNT;
  int idx = *DREF(simple_toggle) ? 1 : 0; // being verbose to make indexing clear

  *DREF(factor_events) = dtask_run((dtask_state_t *)&factor_state[idx], initial);
  return *DREF(factor_events) != 0;
}

DTASK_ENABLE(simple_toggle)
{
  // deoendency on simple_state.toggle cannot be handled automatically
  // because it is within another (sub)graph
  dtask_enable((dtask_state_t *)&simple_state, TOGGLE);
}

DTASK_DISABLE(simple_toggle)
{
  // deoendency on simple_state.toggle cannot be handled automatically
  // because it is within another (sub)graph
  dtask_clear((dtask_state_t *)&simple_state, TOGGLE);
}

DTASK(simple_toggle, bool)
{
  dtask_set_t initial = TOGGLE;

  dtask_set_t simple_events = dtask_run((dtask_state_t *)&simple_state, initial);

  *DREF(simple_toggle) = simple_state.toggle;

  return (simple_events & TOGGLE) != 0;
}
