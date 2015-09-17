#include <stdio.h>

// #define ALL_TASKS_AFTER_TASK_HOOK printf("hook\n")
#ifndef DTASK_GEN
#include "toplevel_tasks.h"
#include "factor_tasks.h"
#include "simple_tasks.h"
#endif

toplevel_tasks_state_t toplevel_state = DTASK_INITIAL_STATE(toplevel_tasks, NULL, 0);
factor_tasks_state_t factor_state[2] = {
  DTASK_INITIAL_STATE(factor_tasks, &toplevel_tasks, 0),
  DTASK_INITIAL_STATE(factor_tasks, &toplevel_tasks, 1)
};
simple_tasks_state_t simple_state = DTASK_INITIAL_STATE(simple_tasks, &toplevel_tasks, 0);

int main() {

  dtask_state_t *state = (dtask_state_t *)&toplevel_state;

  dtask_enable(state, FACTOR_EVENTS | SIMPLE_TOGGLE);

  factor_state[0].count = -1;
  factor_state[1].count = -1;

  printf("\n\n_____ OUTPUT35[0] & OUTPUT57[1] _____\n");
  dtask_switch((dtask_state_t *)&factor_state[0], OUTPUT35);
  dtask_switch((dtask_state_t *)&factor_state[1], OUTPUT57);
  for(int i = 0; i < 100; i++) {
    dtask_run(state, SIMPLE_TOGGLE);
  }

  printf("\n\n_____ OUTPUT357[0] _____\n");
  dtask_switch((dtask_state_t *)&factor_state[0], OUTPUT357);
  dtask_switch((dtask_state_t *)&factor_state[1], 0);

  for(int i = 0; i < 500; i++) {
    dtask_run(state, SIMPLE_TOGGLE);
  }

  printf("\n\n_____ disabling MOD_SEVEN[0] _____\n");
  dtask_switch((dtask_state_t *)&factor_state[0], OUTPUT35 | OUTPUT57);
  dtask_select((dtask_state_t *)&factor_state[0]);
  dtask_disable((dtask_state_t *)&factor_state[0], MOD_SEVEN);
  dtask_select((dtask_state_t *)&factor_state[0]);
  for(int i = 0; i < 100; i++) {
    dtask_run(state, SIMPLE_TOGGLE);
  }

  printf("\n\n_____ clearing MOD_SEVEN[0] _____\n");
  dtask_clear((dtask_state_t *)&factor_state[0], MOD_SEVEN);
  for(int i = 0; i < 100; i++) {
    dtask_run(state, SIMPLE_TOGGLE);
  }

  printf("\n\n_____ disabling OUTPUT35[0] _____\n");
  dtask_switch((dtask_state_t *)&factor_state[0], OUTPUT35 | OUTPUT57);
  dtask_disable((dtask_state_t *)&factor_state[0], OUTPUT35);
  for(int i = 0; i < 100; i++) {
    dtask_run(state, SIMPLE_TOGGLE);
  }

  printf("\n\n_____ FIZZBUZZ[0,1], reset count[0] _____\n");
  dtask_switch((dtask_state_t *)&factor_state[0], FIZZBUZZ);
  dtask_switch((dtask_state_t *)&factor_state[1], FIZZBUZZ);
  factor_state[0].count = 0;
  for(int i = 0; i <= 100; i++) {
    dtask_run(state, SIMPLE_TOGGLE);
  }

  printf("\n\n_____ FIZZBUZZWOOF[1], reset count[1] _____\n");
  dtask_switch((dtask_state_t *)&factor_state[1], FIZZBUZZ | MOD_SEVEN);
  factor_state[1].count = 0;
  for(int i = 0; i <= 110; i++) {
    dtask_run(state, SIMPLE_TOGGLE);
  }

  printf("\n\n_____ clearing FIZZBUZZ[1] _____\n");
  dtask_clear((dtask_state_t *)&factor_state[1], FIZZBUZZ);

  printf("\n\n_____ THE END _____\n\n");

  return 0;
}
