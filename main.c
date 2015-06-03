#include <stdio.h>
#include <unistd.h>

#include "dtask.h"

#define LENGTH(a) (sizeof(a) / sizeof((a)[0]))

int main() {
  const dtask_t tasks[] = ALL_TASKS;
  dtask_state_t state = {
    .tasks = tasks,
    .num_tasks = LENGTH(tasks),
    .enabled = 0
  };

  printf("\n\nenabling OUTPUT35\n");
  sleep(3);
  dtask_disable_all(&state);
  dtask_enable(&state, OUTPUT35);

  for(int i = 0; i < 100; i++) {
    dtask_run(&state);
  }

  printf("\n\nenabling OUTPUT57\n");
  sleep(3);
  dtask_disable_all(&state);
  dtask_enable(&state, OUTPUT57);

  for(int i = 0; i < 100; i++) {
    dtask_run(&state);
  }

  printf("\n\nenabling OUTPUT357\n");
  sleep(3);
  dtask_disable_all(&state);
  dtask_enable(&state, OUTPUT357);

  for(int i = 0; i < 100; i++) {
    dtask_run(&state);
  }

  printf("\n\ndisabling MOD_SEVEN\n");
  dtask_enable(&state, OUTPUT35 | OUTPUT57);
  dtask_disable(&state, MOD_SEVEN);
  printf("enabled = 0x%x\n", state.enabled);
  for(int i = 0; i < 100; i++) {
    dtask_run(&state);
  }

  return 0;
}
