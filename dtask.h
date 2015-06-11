#ifndef __DTASK__
#define __DTASK__

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t dtask_set_t;
typedef uint8_t dtask_id_t;

typedef struct dtask dtask_t;
struct dtask
{
  dtask_set_t all_dependencies, all_dependents;
};

typedef struct dtask_state
{
  const dtask_t *tasks;
  const dtask_id_t num_tasks;
  dtask_set_t enabled, enabled_dependencies;
} dtask_state_t;

void dtask_enable(dtask_state_t *state, dtask_set_t set);
void dtask_disable(dtask_state_t *state, dtask_set_t set);
void dtask_disable_all(dtask_state_t *state);

#define DGET(x) (x)

#define DTASK(name, ...)  \
  name##_t name;          \
  bool __dtask_##name(dtask_set_t events)

#define DECLARE_DTASK(name, type...) \
  typedef type name##_t;             \
  extern name##_t name;              \
  bool __dtask_##name(dtask_set_t events)

#define DTASK_AND(x) (!(~events & (x)))
#define DTASK_OR(x) (events & (x))

// define NO_CLZ for targets that do not efficiently implement __builtin_clz()
#if defined(__TARGET_CPU_CORTEX_M0) || defined(__TARGET_CPU_CORTEX_M0PLUS)
#define NO_CLZ
#endif

#endif
