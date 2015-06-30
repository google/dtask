#include <stdio.h>
#include <assert.h>

#ifndef GEN
#include "stage1_tasks.h"
#include "stage2_tasks.h"
#endif

#define DEBUG 1

#if DEBUG
#define debugf(args...) printf(args)
#else
#define debugf(...)
#endif

DTASK_GROUP(stage1_tasks)

DTASK_ENABLE(fizzbuzz) {
  printf("fizzbuzz enabled\n");
}
DTASK_DISABLE(fizzbuzz) {
  printf("fizzbuzz disabled\n");
}
DTASK(fizzbuzz, int) {
  int x = DGET(count);
  (void)DGET(mod_three);
  (void)DGET(mod_five);
  if(DTASK_AND(MOD_THREE | MOD_FIVE)) {
    printf("fizzbuzz\n");
  } else if(events & MOD_THREE) {
    printf("fizz\n");
  } else if(events & MOD_FIVE) {
    printf("buzz\n");
  } else {
    printf("%d\n", x);
  }
  return true;
}

DTASK_ENABLE(output35) {
  printf("output35 enabled\n");
}
DTASK_DISABLE(output35) {
  printf("output35 disabled\n");
}
DTASK(output35, int) {
  const combine35_t *x = &DGET(combine35);
  assert(x->m3 * 3 == x->m5 * 5);
  debugf("\n");
  printf("%d * 3 == %d * 5 == %d\n", x->m3, x->m5, x->m3 * 3);
  debugf("\n");
  return true;
}

DTASK_ENABLE(output57) {
  printf("output57 enabled\n");
}
DTASK_DISABLE(output57) {
  printf("output57 disabled\n");
}
DTASK(output57, int) {
  const combine57_t *x = &DGET(combine57);
  assert(x->m5 * 5 == x->m7 * 7);
  debugf("\n");
  printf("%d * 5 == %d * 7 == %d\n", x->m5, x->m7, x->m5 * 5);
  debugf("\n");
  return true;
}

DTASK_ENABLE(output357) {
  printf("output357 enabled\n");
}
DTASK_DISABLE(output357) {
  printf("output357 disabled\n");
}
DTASK(output357, int) {
  const combine357_t *x = &DGET(combine357);
  assert(x->m3 * 3 == x->m5 * 5 && x->m5 * 5 == x->m7 * 7);
  debugf("\n");
  printf("%d * 3 == %d * 5 == %d * 7 == %d\n", x->m3, x->m5, x->m7, x->m3 * 3);
  debugf("\n");
  return true;
}

DTASK_ENABLE(combine357) {
  printf("combine357 enabled\n");
}
DTASK_DISABLE(combine357) {
  printf("combine357 disabled\n");
}
DTASK(combine357, struct { int m3, m5, m7; }) {
  const combine35_t *m35 = &DGET(combine35);
  int m7 = DGET(mod_seven);
  if(DTASK_AND(COMBINE35 | MOD_SEVEN)) {
    combine357.m3 = m35->m3;
    combine357.m5 = m35->m5;
    combine357.m7 = m7;
    return true;
  } else {
    return false;
  }
}

DTASK_ENABLE(combine35) {
  printf("combine35 enabled\n");
}
DTASK_DISABLE(combine35) {
  printf("combine35 disabled\n");
}
DTASK(combine35, struct { int m3, m5; }) {
  int m3 = DGET(mod_three);
  int m5 = DGET(mod_five);
  if(DTASK_AND(MOD_THREE | MOD_FIVE)) {
    combine35.m3 = m3;
    combine35.m5 = m5;
    return true;
  } else {
    return false;
  }
}

DTASK_ENABLE(combine57) {
  printf("combine57 enabled\n");
}
DTASK_DISABLE(combine57) {
  printf("combine57 disabled\n");
}
DTASK(combine57, struct { int m5, m7; }) {
  int m5 = DGET(mod_five);
  int m7 = DGET(mod_seven);
  if(DTASK_AND(MOD_FIVE | MOD_SEVEN)) {
    combine57.m5 = m5;
    combine57.m7 = m7;
    return true;
  } else {
    return false;
  }
}


DTASK_ENABLE(mod_three) {
  printf("mod_three enabled\n");
}
DTASK_DISABLE(mod_three) {
  printf("mod_three disabled\n");
}
DTASK(mod_three, int) {
  int x = DGET(count);
  if(x % 3 != 0) return false;
  mod_three = x / 3;
  debugf("[3] ");
  return true;
}

DTASK_ENABLE(mod_five) {
  printf("mod_five enabled\n");
}
DTASK_DISABLE(mod_five) {
  printf("mod_five disabled\n");
}
DTASK(mod_five, int) {
  int x = DGET(count);
  if(x % 5 != 0) return false;
  mod_five = x / 5;
  debugf("[5] ");
  return true;
}

DTASK_ENABLE(mod_seven) {
  printf("mod_seven enabled\n");
}
DTASK_DISABLE(mod_seven) {
  printf("mod_seven disabled\n");
}
DTASK(mod_seven, int) {
  int x = DGET(count);
  if(x % 7 != 0) return false;
  mod_seven = x / 7;
  debugf("[7] ");
  return true;
}

DTASK(count, int) {
  count++;
  return true;
}

DTASK_GROUP(stage2_tasks)

DTASK(toggle, bool) {
  toggle = !toggle;
  return true;
}
