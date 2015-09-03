#include <stdio.h>

// #define ALL_TASKS_AFTER_TASK_HOOK printf("hook\n")
#ifndef DTASK_GEN
#include "toplevel_tasks.h"
#include "factor_tasks.h"
#include "simple_tasks.h"
#endif

dtask_state_t toplevel = DTASK_INITIAL_STATE(toplevel_tasks);
dtask_state_t factor = DTASK_INITIAL_STATE(factor_tasks);
dtask_state_t simple = DTASK_INITIAL_STATE(simple_tasks);

toplevel_tasks_data_t toplevel_data;
factor_tasks_data_t factor_data;
simple_tasks_data_t simple_data;

int main() {

  dtask_enable(&toplevel, FACTOR_EVENTS | SIMPLE_EVENTS);
  dtask_enable(&simple, TOGGLE | TOGGLE2);

  factor_data.count = -1;

  printf("\n\n_____ OUTPUT35 _____\n");
  dtask_switch(&factor, OUTPUT35);

  for(int i = 0; i < 100; i++) {
    dtask_run(&toplevel, &toplevel_data, SIMPLE_EVENTS, 0);
  }

  printf("\n\n_____ OUTPUT57 _____\n");
  dtask_switch(&factor, OUTPUT57);

  for(int i = 0; i < 100; i++) {
    dtask_run(&toplevel, &toplevel_data, SIMPLE_EVENTS, 0);
  }

  printf("\n\n_____ OUTPUT357 _____\n");
  dtask_switch(&factor, OUTPUT357);

  for(int i = 0; i < 500; i++) {
    dtask_run(&toplevel, &toplevel_data, SIMPLE_EVENTS, 0);
  }

  printf("\n\n_____ disabling MOD_SEVEN _____\n");
  dtask_switch(&factor, OUTPUT35 | OUTPUT57);
  dtask_disable(&factor, MOD_SEVEN);
  for(int i = 0; i < 100; i++) {
    dtask_run(&toplevel, &toplevel_data, SIMPLE_EVENTS, 0);
  }

  printf("\n\n_____ clearing MOD_SEVEN _____\n");
  dtask_clear(&factor, MOD_SEVEN);
  for(int i = 0; i < 100; i++) {
    dtask_run(&toplevel, &toplevel_data, SIMPLE_EVENTS, 0);
  }

  printf("\n\n_____ disabling OUTPUT35 _____\n");
  dtask_switch(&factor, OUTPUT35 | OUTPUT57);
  dtask_disable(&factor, OUTPUT35);
  for(int i = 0; i < 100; i++) {
    dtask_run(&toplevel, &toplevel_data, SIMPLE_EVENTS, 0);
  }

  printf("\n\n_____ FIZZBUZZ _____\n");
  dtask_switch(&factor, FIZZBUZZ);
  factor_data.count = 0;
  for(int i = 0; i <= 100; i++) {
    dtask_run(&toplevel, &toplevel_data, SIMPLE_EVENTS, 0);
  }

  printf("\n\n_____ FIZZBUZZWOOF _____\n");
  dtask_switch(&factor, FIZZBUZZ | MOD_SEVEN);
  factor_data.count = 0;
  for(int i = 0; i <= 110; i++) {
    dtask_run(&toplevel, &toplevel_data, SIMPLE_EVENTS, 0);
  }

  printf("\n\n_____ clearing FIZZBUZZ _____\n");
  dtask_clear(&factor, FIZZBUZZ);

  printf("\n\n_____ THE END _____\n\n");

  return 0;
}
