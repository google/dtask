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

typedef struct dtask_state dtask_state_t;
typedef struct dtask dtask_t;
typedef struct dtask_config dtask_config_t;
typedef struct dtask_select dtask_select_t;

struct dtask
{
#ifndef NO_CLZ
  bool (*func)(dtask_state_t *state);
#endif
  void (*enable_func)(dtask_state_t *state);
  void (*disable_func)(dtask_state_t *state);
#ifndef NO_CLZ
  dtask_set_t dependents;
#endif
  dtask_set_t all_dependencies, all_dependents;
};

#define DTASK_STATE_HEADER              \
  struct                                \
  {                                     \
    dtask_set_t events;                 \
    dtask_config_t config;              \
    dtask_select_t select;              \
  }

struct dtask_config
{
  const dtask_t *tasks;
  const dtask_state_t *parent;
#ifdef NO_CLZ
  dtask_set_t (*const run)(dtask_state_t *state, dtask_set_t initial);
#endif
  const unsigned int id;
};

struct dtask_select
{
  dtask_set_t
    enabled,
    disabled,
    enabled_dependencies,
    disabled_dependents,
    requested,
    selected;
};

struct dtask_state
{
  DTASK_STATE_HEADER;
};

dtask_set_t dtask_run(dtask_state_t *state, dtask_set_t initial);
void dtask_enable(dtask_state_t *state, dtask_set_t set);
void dtask_disable(dtask_state_t *state, dtask_set_t set);
void dtask_clear(dtask_state_t *state, dtask_set_t set);
void dtask_switch(dtask_state_t *state, dtask_set_t set);
void __dtask_noop(dtask_state_t *state);

#ifndef DTASK_GEN

#define DTASK(name, ...)  \
  bool __dtask_##name(dtask_state_t *state)

#define DTASK_GROUP(group_name) \
typedef group_name##_state_t dref_t;

#define DREF(x) ((x##_t *)&((dref_t *)state)->x)
#define DREF_WEAK(x) DREF(x)

#define DTASK_ENABLE(name)  \
  void __dtask_enable_##name(dtask_state_t *state)

#define DTASK_DISABLE(name)  \
  void __dtask_disable_##name(dtask_state_t *state)

#endif

#define DECLARE_DTASK(name, type...) \
  typedef type name##_t;             \
  bool __dtask_##name(dtask_state_t *state)

#define DECLARE_DTASK_ENABLE(name) \
  void __dtask_enable_##name(dtask_state_t *state)

#define DECLARE_DTASK_DISABLE(name) \
  void __dtask_disable_##name(dtask_state_t *state)

#define DTASK_AND(x) (!(~state->events & (x)))
#define DTASK_OR(x) (state->events & (x))
#define DTASK_LENGTH(a) (sizeof(a) / sizeof((a)[0]))

#ifndef NO_CLZ
#define DTASK_INITIAL_CONFIG(name, parent, id) {(name), (dtask_state_t *)(parent), (id)}
#else
#define DTASK_INITIAL_CONFIG(name, parent, id) {(name), (dtask_state_t *)(parent), name##_run, (id)}
#endif

#define DTASK_INITIAL_STATE(name, parent, id) { .config = DTASK_INITIAL_CONFIG(name, parent, id) }

#define DTASK_BIT_WIDTH(type) (sizeof(type) * 8)

#define DTASK_MAX_ID (DTASK_BIT_WIDTH(dtask_set_t) - 1)

static inline
dtask_set_t dtask_bit(dtask_id_t id) {
  return (1U << DTASK_MAX_ID) >> id;
}


#endif
