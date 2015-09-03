#include <stdio.h>

// #define ALL_TASKS_AFTER_TASK_HOOK printf("hook\n")
#ifndef DTASK_GEN
#include "toplevel_tasks.h"
#include "factor_tasks.h"
#include "simple_tasks.h"
#endif

dtask_config_t toplevel_config = DTASK_INITIAL_CONFIG(toplevel_tasks);
dtask_config_t factor_config = DTASK_INITIAL_CONFIG(factor_tasks);
dtask_config_t simple_config = DTASK_INITIAL_CONFIG(simple_tasks);

toplevel_tasks_state_t toplevel_state;
factor_tasks_state_t factor_state[2];
simple_tasks_state_t simple_state;

int main() {

  dtask_enable(&toplevel_config, FACTOR_EVENTS | SIMPLE_TOGGLE);
  dtask_enable(&simple_config, TOGGLE);

  factor_state[0].count = -1;
  factor_state[1].count = -1;
  factor_state[0].id = 0;
  factor_state[1].id = 1;

  printf("\n\n_____ OUTPUT35 _____\n");
  dtask_switch(&factor_config, OUTPUT35);

  for(int i = 0; i < 100; i++) {
    dtask_run(&toplevel_config, &toplevel_state, SIMPLE_TOGGLE, 0);
  }

  printf("\n\n_____ OUTPUT57 _____\n");
  dtask_switch(&factor_config, OUTPUT57);

  for(int i = 0; i < 100; i++) {
    dtask_run(&toplevel_config, &toplevel_state, SIMPLE_TOGGLE, 0);
  }

  printf("\n\n_____ OUTPUT357 _____\n");
  dtask_switch(&factor_config, OUTPUT357);

  for(int i = 0; i < 500; i++) {
    dtask_run(&toplevel_config, &toplevel_state, SIMPLE_TOGGLE, 0);
  }

  printf("\n\n_____ disabling MOD_SEVEN _____\n");
  dtask_switch(&factor_config, OUTPUT35 | OUTPUT57);
  dtask_disable(&factor_config, MOD_SEVEN);
  for(int i = 0; i < 100; i++) {
    dtask_run(&toplevel_config, &toplevel_state, SIMPLE_TOGGLE, 0);
  }

  printf("\n\n_____ clearing MOD_SEVEN _____\n");
  dtask_clear(&factor_config, MOD_SEVEN);
  for(int i = 0; i < 100; i++) {
    dtask_run(&toplevel_config, &toplevel_state, SIMPLE_TOGGLE, 0);
  }

  printf("\n\n_____ disabling OUTPUT35 _____\n");
  dtask_switch(&factor_config, OUTPUT35 | OUTPUT57);
  dtask_disable(&factor_config, OUTPUT35);
  for(int i = 0; i < 100; i++) {
    dtask_run(&toplevel_config, &toplevel_state, SIMPLE_TOGGLE, 0);
  }

  printf("\n\n_____ FIZZBUZZ _____\n");
  dtask_switch(&factor_config, FIZZBUZZ);
  factor_state[0].count = 0;
  for(int i = 0; i <= 100; i++) {
    dtask_run(&toplevel_config, &toplevel_state, SIMPLE_TOGGLE, 0);
  }

  printf("\n\n_____ FIZZBUZZWOOF _____\n");
  dtask_switch(&factor_config, FIZZBUZZ | MOD_SEVEN);
  factor_state[0].count = 0;
  for(int i = 0; i <= 110; i++) {
    dtask_run(&toplevel_config, &toplevel_state, SIMPLE_TOGGLE, 0);
  }

  printf("\n\n_____ clearing FIZZBUZZ _____\n");
  dtask_clear(&factor_config, FIZZBUZZ);

  printf("\n\n_____ THE END _____\n\n");

  return 0;
}
