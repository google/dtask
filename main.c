#include <stdio.h>

// #define ALL_TASKS_AFTER_TASK_HOOK printf("hook\n")
#ifndef GEN
#include "stage1_tasks.h"
#include "stage2_tasks.h"
#endif

dtask_state_t state1 = DTASK_INITIAL_STATE(stage1_tasks);
dtask_state_t state2 = DTASK_INITIAL_STATE(stage2_tasks);

int main() {

  count = -1;

  printf("\n\n_____ OUTPUT35 _____\n");
  dtask_switch(&state1, OUTPUT35);

  for(int i = 0; i < 100; i++) {
    dtask_run(&state1, COUNT);
  }

  printf("\n\n_____ OUTPUT57 _____\n");
  dtask_switch(&state1, OUTPUT57);

  for(int i = 0; i < 100; i++) {
    dtask_run(&state1, COUNT);
  }

  printf("\n\n_____ OUTPUT357 _____\n");
  dtask_switch(&state1, OUTPUT357);

  for(int i = 0; i < 500; i++) {
    dtask_run(&state1, COUNT);
  }

  printf("\n\n_____ disabling MOD_SEVEN _____\n");
  dtask_switch(&state1, OUTPUT35 | OUTPUT57);
  dtask_disable(&state1, MOD_SEVEN);
  for(int i = 0; i < 100; i++) {
    dtask_run(&state1, COUNT);
  }

  printf("\n\n_____ disabling OUTPUT35 _____\n");
  dtask_switch(&state1, OUTPUT35 | OUTPUT57);
  dtask_disable(&state1, OUTPUT35);
  for(int i = 0; i < 100; i++) {
    dtask_run(&state1, COUNT);
  }

  printf("\n\n_____ FIZZBUZZ _____\n");
  dtask_switch(&state1, FIZZBUZZ);
  count = 0;
  for(int i = 0; i <= 100; i++) {
    dtask_run(&state1, COUNT);
  }

  printf("\n\n_____ THE END _____\n\n");

  return 0;
}
