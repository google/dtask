#include <stdio.h>
#include <assert.h>
#include "types.h"

#ifndef DTASK_GEN
#include "factor_tasks.h"
#endif

#define DEBUG 1

#if DEBUG
#define debugf(args...) printf(args)
#else
#define debugf(...)
#endif

DTASK_GROUP(factor_tasks)

DTASK_ENABLE(fizzbuzz) {
  printf("%d > fizzbuzz enabled\n", state->config.id);
}
DTASK_DISABLE(fizzbuzz) {
  printf("%d > fizzbuzz disabled\n", state->config.id);
}
DTASK(fizzbuzz, int) {
  int x = *DREF(count);
  (void)DREF(mod_three);
  (void)DREF(mod_five);
  (void)DREF_WEAK(mod_seven);
  printf("%d > ", state->config.id);
  if(DTASK_OR(MOD_THREE | MOD_FIVE | MOD_SEVEN)) {
    if(state->events & MOD_THREE) {
      printf("fizz");
    }
    if(state->events & MOD_FIVE) {
      printf("buzz");
    }
    if(state->events & MOD_SEVEN) {
      printf("woof");
    }
    printf("\n");
  } else {
    printf("%d\n", x);
  }
  return true;
}

DTASK_ENABLE(output35) {
  printf("%d > output35 enabled\n", state->config.id);
}
DTASK_DISABLE(output35) {
  printf("%d > output35 disabled\n", state->config.id);
}
DTASK(output35, int) {
  const combine35_t *x = DREF(combine35);
  assert(x->m3 * 3 == x->m5 * 5);
  debugf("\n");
  printf("%d > ", state->config.id);
  printf("%d * 3 == %d * 5 == %d\n", x->m3, x->m5, x->m3 * 3);
  debugf("\n");
  return true;
}

DTASK_ENABLE(output57) {
  printf("%d > output57 enabled\n", state->config.id);
}
DTASK_DISABLE(output57) {
  printf("%d > output57 disabled\n", state->config.id);
}
DTASK(output57, int) {
  const combine57_t *x = DREF(combine57);
  assert(x->m5 * 5 == x->m7 * 7);
  debugf("\n");
  printf("%d > ", state->config.id);
  printf("%d * 5 == %d * 7 == %d\n", x->m5, x->m7, x->m5 * 5);
  debugf("\n");
  return true;
}

DTASK_ENABLE(output357) {
  printf("%d > output357 enabled\n", state->config.id);
}
DTASK_DISABLE(output357) {
  printf("%d > output357 disabled\n", state->config.id);
}
DTASK(output357, int) {
  const combine357_t *x = DREF(combine357);
  assert(x->m3 * 3 == x->m5 * 5 && x->m5 * 5 == x->m7 * 7);
  debugf("\n");
  printf("%d > ", state->config.id);
  printf("%d * 3 == %d * 5 == %d * 7 == %d\n", x->m3, x->m5, x->m7, x->m3 * 3);

  // demonstrate delays
  printf("d = [%d", *DELAY_READ(&x->d, int, OUTPUT_DELAY_SIZE, 0));
  for(int i = 1; i < OUTPUT_DELAY_SIZE; i++)
  {
    int v = *DELAY_READ(&x->d, int, OUTPUT_DELAY_SIZE, i);
    printf(", %d", v);
  }
  printf("]\n");
  debugf("\n");
  return true;
}

DTASK_ENABLE(combine357) {
  printf("%d > combine357 enabled\n", state->config.id);
  int zero = 0;
  DELAY_FILL(&DREF(combine357)->d, int, OUTPUT_DELAY_SIZE, &zero);
}
DTASK_DISABLE(combine357) {
  printf("%d > combine357 disabled\n", state->config.id);
}
DTASK(combine357, struct { int m3, m5, m7; DELAY(int, OUTPUT_DELAY_SIZE) d; }) {
  const combine35_t *m35 = DREF(combine35);
  int m7 = *DREF(mod_seven);
  if(DTASK_AND(COMBINE35 | MOD_SEVEN)) {
    DREF(combine357)->m3 = m35->m3;
    DREF(combine357)->m5 = m35->m5;
    DREF(combine357)->m7 = m7;

    // demonstrate delays
    DELAY_WRITE(&DREF(combine357)->d, int, OUTPUT_DELAY_SIZE, DREF_WEAK(count));
    return true;
  } else {
    return false;
  }
}

DTASK_ENABLE(combine35) {
  printf("%d > combine35 enabled\n", state->config.id);
}
DTASK_DISABLE(combine35) {
  printf("%d > combine35 disabled\n", state->config.id);
}
DTASK(combine35, struct { int m3, m5; }) {
  int m3 = *DREF(mod_three);
  int m5 = *DREF(mod_five);
  if(DTASK_AND(MOD_THREE | MOD_FIVE)) {
    DREF(combine35)->m3 = m3;
    DREF(combine35)->m5 = m5;
    return true;
  } else {
    return false;
  }
}

DTASK_ENABLE(combine57) {
  printf("%d > combine57 enabled\n", state->config.id);
}
DTASK_DISABLE(combine57) {
  printf("%d > combine57 disabled\n", state->config.id);
}
DTASK(combine57, struct { int m5, m7; }) {
  int m5 = *DREF(mod_five);
  int m7 = *DREF(mod_seven);
  if(DTASK_AND(MOD_FIVE | MOD_SEVEN)) {
    DREF(combine57)->m5 = m5;
    DREF(combine57)->m7 = m7;
    return true;
  } else {
    return false;
  }
}


DTASK_ENABLE(mod_three) {
  printf("%d > mod_three enabled\n", state->config.id);
}
DTASK_DISABLE(mod_three) {
  printf("%d > mod_three disabled\n", state->config.id);
}
DTASK(mod_three, int) {
  int x = *DREF(count);
  if(x % 3 != 0) return false;
  *DREF(mod_three) = x / 3;
  debugf("[3] ");
  return true;
}

DTASK_ENABLE(mod_five) {
  printf("%d > mod_five enabled\n", state->config.id);
}
DTASK_DISABLE(mod_five) {
  printf("%d > mod_five disabled\n", state->config.id);
}
DTASK(mod_five, int) {
  int x = *DREF(count);
  if(x % 5 != 0) return false;
  *DREF(mod_five) = x / 5;
  debugf("[5] ");
  return true;
}

DTASK_ENABLE(mod_seven) {
  printf("%d > mod_seven enabled\n", state->config.id);
}
DTASK_DISABLE(mod_seven) {
  printf("%d > mod_seven disabled\n", state->config.id);
}
DTASK(mod_seven, int) {
  int x = *DREF(count);
  if(x % 7 != 0) return false;
  *DREF(mod_seven) = x / 7;
  debugf("[7] ");
  return true;
}

DTASK(count, int) {
  (*DREF(count))++;
  return true;
}
