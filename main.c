#include <stdio.h>

#include "dtask.h"

int main() {
  dtask_t tasks[] = ALL_TASKS;
  dtask_run(tasks);

  return 0;
}
