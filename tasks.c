#include <stdio.h>
#include "dtask.h"

DTASK(count, int) {
  int x = DGET(count);
  if(x > 100) return false;
  count = x + 1;
  printf("count -> %d\n", count);
  return true;
}

DTASK(mod_three, int) {
  int x = DGET(count);
  if(x % 3 != 0) return false;
  mod_three = x;
  printf("mod_three -> %d\n", mod_three);
  return true;
}

DTASK(mod_five, int) {
  int x = DGET(count);
  if(x % 5 != 0) return false;
  mod_five = x;
  printf("mod_five -> %d\n", mod_five);
  return true;
}

DTASK(output, int) {
  int m3 = DGET(mod_three);
  int m5 = DGET(mod_five);
  output = mask & (1 << MOD_THREE) ? m3 : m5;
  printf("output -> %d\n", output);
  return true;
}
