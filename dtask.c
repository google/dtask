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

static void dtask_update_enabled_dependencies(dtask_config_t *config, dtask_set_t set) {
  config->enabled |= set;
  dtask_set_t enabled_dependencies = config->enabled_dependencies | set;
  const dtask_t *tasks = config->tasks;
  dtask_id_t n = 0;

  // enable dependencies
  while(set) {
    n = dtask_set_find_first(set, n);
    enabled_dependencies |= tasks[n].all_dependencies;
    set &= ~dtask_bit(n);
  }

  config->enabled_dependencies = enabled_dependencies;
}

static void dtask_update_disabled_dependents(dtask_config_t *config, dtask_set_t set) {
  config->disabled |= set;
  dtask_set_t disabled_dependents = config->disabled_dependents | set;
  const dtask_t *tasks = config->tasks;
  dtask_id_t n = 0;

  // disable dependents
  while(set) {
    n = dtask_set_find_first(set, n);
    disabled_dependents |= tasks[n].all_dependents;
    set &= ~dtask_bit(n);
  }

  config->disabled_dependents = disabled_dependents;
}

static void dtask_call_enable_functions(dtask_config_t *config, dtask_set_t prev_selected) {
  dtask_set_t new_enabled = config->selected & ~prev_selected;
  const dtask_t *tasks = config->tasks;
  dtask_id_t n = 0;

  // call enable functions
  while(new_enabled) {
    n = dtask_set_find_first(new_enabled, n);
    tasks[n].enable_func();
    new_enabled &= ~dtask_bit(n);
  }
}

static void dtask_call_disable_functions(dtask_config_t *config, dtask_set_t prev_selected) {
  dtask_set_t new_disabled = ~config->selected & prev_selected;
  const dtask_t *tasks = config->tasks;
  dtask_id_t n = DTASK_MAX_ID;

  // call disable functions (in reverse order, before dependencies)
  while(new_disabled) {
    n = dtask_set_find_last(new_disabled, n);
    tasks[n].disable_func();
    new_disabled &= ~dtask_bit(n);
  }
}

void dtask_enable(dtask_config_t *config, dtask_set_t set)
{
  dtask_set_t prev_selected = config->selected;
  dtask_update_enabled_dependencies(config, set);

  // clear conflicting disables and recompute
  if(config->disabled & config->enabled_dependencies) {
    config->disabled &= ~config->enabled_dependencies;
    config->disabled_dependents = 0;
    dtask_update_disabled_dependents(config, config->disabled);
  }

  // update selected and call enable functions
  config->selected = config->enabled_dependencies & ~config->disabled_dependents;
  dtask_call_enable_functions(config, prev_selected);
}

void dtask_disable(dtask_config_t *config, dtask_set_t set)
{
  dtask_set_t prev_selected = config->selected;
  dtask_update_disabled_dependents(config, set);

  // clear directly conflicting enables and recompute
  if(config->enabled & set) {
    config->enabled &= ~set;
    config->enabled_dependencies = 0;
    dtask_update_enabled_dependencies(config, config->enabled);
  }

  // update selected and call disable functions
  config->selected = config->enabled_dependencies & ~config->disabled_dependents;
  dtask_call_disable_functions(config, prev_selected);
}

void dtask_clear(dtask_config_t *config, dtask_set_t set) {
  dtask_set_t prev_selected = config->selected;

  // clear directly conflicting enables and recompute
  if(config->enabled & set) {
    config->enabled &= ~set;
    config->enabled_dependencies = 0;
    dtask_update_enabled_dependencies(config, config->enabled);
  }

  // clear directly conflicting disables and recompute
  if(config->disabled & set) {
    config->disabled &= ~set;
    config->disabled_dependents = 0;
    dtask_update_disabled_dependents(config, config->disabled);
  }

  // update selected and call disable/enable functions
  config->selected = config->enabled_dependencies & ~config->disabled_dependents;
  dtask_call_disable_functions(config, prev_selected);
  dtask_call_enable_functions(config, prev_selected);
}

void dtask_switch(dtask_config_t *config, dtask_set_t set) {
  dtask_set_t prev_selected = config->selected;

  // reset enable/disable config
  config->enabled = 0;
  config->disabled = 0;
  config->enabled_dependencies = 0;
  config->disabled_dependents = 0;

  // enable set
  dtask_update_enabled_dependencies(config, set);
  config->selected = config->enabled_dependencies;

  // call enable/disable functions
  dtask_call_disable_functions(config, prev_selected);
  dtask_call_enable_functions(config, prev_selected);
}

#ifndef NO_CLZ
dtask_set_t dtask_run(const dtask_config_t *config, void *state, dtask_set_t initial, dtask_set_t parent_events) {
  const dtask_set_t selected = config->selected;
  dtask_set_t
    scheduled = initial & selected,
    events = 0,
    active = 0;

  while(scheduled) {
    active = dtask_set_find_first(scheduled, active);
    const dtask_set_t id_bit = dtask_bit(active);
    const dtask_t *task = &config->tasks[active];
    if((id_bit & scheduled) &&
       task->func(state, events, parent_events)) {
      events |= id_bit;
      scheduled |= task->dependents & selected;
    }
    scheduled &= ~id_bit;
  }
  return events;
}
#else
dtask_set_t dtask_run(const dtask_config_t *config, void *state, dtask_set_t initial, dtask_set_t parent_events) {
  return config->run(config, state, initial, parent_events);
}
#endif
