#include <stdio.h>

// #define ALL_TASKS_AFTER_TASK_HOOK printf("hook\n")
#include "all_tasks.h"

int main() {
  dtask_state_t state = DTASK_INITIAL_STATE(all_tasks);

  count = -1;

  printf("\n\n_____ enabling OUTPUT35 _____\n");
  dtask_disable_all(&state);
  dtask_enable(&state, OUTPUT35);

  for(int i = 0; i < 100; i++) {
    dtask_run(&state, ALL_TASKS_INITIAL);
  }

  printf("\n\n_____ enabling OUTPUT57 _____\n");
  dtask_disable_all(&state);
  dtask_enable(&state, OUTPUT57);

  for(int i = 0; i < 100; i++) {
    dtask_run(&state, ALL_TASKS_INITIAL);
  }

  printf("\n\n_____ enabling OUTPUT357 _____\n");
  dtask_disable_all(&state);
  dtask_enable(&state, OUTPUT357);

  for(int i = 0; i < 500; i++) {
    dtask_run(&state, ALL_TASKS_INITIAL);
  }

  printf("\n\n_____ disabling MOD_SEVEN _____\n");
  dtask_enable(&state, OUTPUT35 | OUTPUT57);
  dtask_disable(&state, MOD_SEVEN);
  for(int i = 0; i < 100; i++) {
    dtask_run(&state, ALL_TASKS_INITIAL);
  }

  printf("\n\n_____ disabling OUTPUT35 _____\n");
  dtask_enable(&state, OUTPUT35 | OUTPUT57);
  dtask_disable(&state, OUTPUT35);
  for(int i = 0; i < 100; i++) {
    dtask_run(&state, ALL_TASKS_INITIAL);
  }

  printf("\n\n_____ FIZZBUZZ _____\n");
  dtask_disable_all(&state);
  dtask_enable(&state, FIZZBUZZ);
  count = 0;
  for(int i = 0; i <= 100; i++) {
    dtask_run(&state, ALL_TASKS_INITIAL);
  }

  printf("\n\n_____ THE END _____\n\n");

  return 0;
}
