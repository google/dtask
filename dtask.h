#ifndef __DTASK__
#define __DTASK__

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t dtask_set_t;
typedef uint8_t dtask_id_t;

// define NO_CLZ for targets that do not efficiently implement __builtin_clz()
#if defined(__TARGET_CPU_CORTEX_M0) || defined(__TARGET_CPU_CORTEX_M0PLUS)
#define NO_CLZ
#endif

typedef struct dtask dtask_t;
struct dtask
{
#ifndef NO_CLZ
  bool (*func)(uint32_t);
  dtask_set_t dependents;
#endif
  dtask_set_t all_dependencies, all_dependents;
};

typedef struct dtask_state dtask_state_t;
struct dtask_state
{
  const dtask_t *tasks;
  const dtask_id_t num_tasks;
  dtask_set_t enabled, enabled_dependencies;
#ifdef NO_CLZ
  void (*run)(const dtask_state_t *state, dtask_set_t initial);
#endif
};

void dtask_run(const dtask_state_t *state, dtask_set_t initial);
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
#define DTASK_LENGTH(a) (sizeof(a) / sizeof((a)[0]))

#ifndef NO_CLZ
#define DTASK_INITIAL_STATE(name) {(name), DTASK_LENGTH(name), 0, 0}
#else
#define DTASK_INITIAL_STATE(name) {(name), DTASK_LENGTH(name), 0, 0, name##_run}
#endif

#define DTASK_BIT_WIDTH(type) (sizeof(type) * 8)

static inline
dtask_set_t dtask_bit(dtask_id_t id) {
  return (1U << (DTASK_BIT_WIDTH(dtask_set_t) - 1)) >> id;
}


#endif
