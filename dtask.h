#ifndef __DTASK__
#define __DTASK__

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t dtask_set_t;
typedef uint8_t dtask_id_t;

// define NO_CLZ for targets that do not efficiently implement __builtin_clz()
#if defined(__TARGET_CPU_CORTEX_M0) || defined(__TARGET_CPU_CORTEX_M0PLUS)
#define NO_CLZ
#define NO_CTZ
#endif

typedef struct dtask dtask_t;
struct dtask
{
#ifndef NO_CLZ
  bool (*func)(void *data, dtask_set_t events, dtask_set_t parent_events);
#endif
  void (*enable_func)(void);
  void (*disable_func)(void);
#ifndef NO_CLZ
  dtask_set_t dependents;
#endif
  dtask_set_t all_dependencies, all_dependents;
};

typedef struct dtask_state dtask_state_t;
struct dtask_state
{
  const dtask_t *tasks;
  const dtask_id_t num_tasks;
  dtask_set_t
    enabled,
    disabled,
    enabled_dependencies,
    disabled_dependents,
    selected;
#ifdef NO_CLZ
  dtask_set_t (*run)(const dtask_state_t *state, void *data, dtask_set_t initial, dtask_set_t parent_events);
#endif
};

dtask_set_t dtask_run(const dtask_state_t *state, void *data, dtask_set_t initial, dtask_set_t parent_events);
void dtask_enable(dtask_state_t *state, dtask_set_t set);
void dtask_disable(dtask_state_t *state, dtask_set_t set);
void dtask_clear(dtask_state_t *state, dtask_set_t set);
void dtask_switch(dtask_state_t *state, dtask_set_t set);
void __dtask_noop(void);

#ifndef DTASK_GEN

#define DTASK(name, ...)  \
  bool __dtask_##name(void *data, dtask_set_t events, dtask_set_t parent_events)

#define DTASK_GROUP(group_name)

#define DTASK_ENABLE(name)  \
  void __dtask_enable_##name(void)

#define DTASK_DISABLE(name)  \
  void __dtask_disable_##name(void)

#endif

#define DECLARE_DTASK(name, type...) \
  typedef type name##_t;             \
  bool __dtask_##name(void *data, dtask_set_t events, dtask_set_t parent_events)

#define DECLARE_DTASK_ENABLE(name) \
  void __dtask_enable_##name(void)

#define DECLARE_DTASK_DISABLE(name) \
  void __dtask_disable_##name(void)

#define DTASK_AND(x) (!(~events & (x)))
#define DTASK_OR(x) (events & (x))
#define DTASK_LENGTH(a) (sizeof(a) / sizeof((a)[0]))

#ifndef NO_CLZ
#define DTASK_INITIAL_STATE(name) {(name), DTASK_LENGTH(name), 0, 0, 0, 0, 0}
#else
#define DTASK_INITIAL_STATE(name) {(name), DTASK_LENGTH(name), 0, 0, 0, 0, 0, name##_run}
#endif

#define DTASK_BIT_WIDTH(type) (sizeof(type) * 8)

#define DTASK_MAX_ID (DTASK_BIT_WIDTH(dtask_set_t) - 1)

static inline
dtask_set_t dtask_bit(dtask_id_t id) {
  return (1U << DTASK_MAX_ID) >> id;
}


#endif
