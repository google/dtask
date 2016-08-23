#ifndef __DTASK__
#define __DTASK__

#include <stdint.h>
#include <stdbool.h>

// define NO_CLZ for targets that do not efficiently implement __builtin_clz()
#if defined(__TARGET_CPU_CORTEX_M0) || defined(__TARGET_CPU_CORTEX_M0PLUS)
#define NO_CLZ
#define NO_CTZ
#endif

/*-------------------- utility macros --------------------*/

// bits in a type
#define DTASK_BIT_WIDTH(type) (sizeof(type) * 8)

// number of elements in an array
#define DTASK_LENGTH(a) (sizeof(a) / sizeof((a)[0]))

/*-------------------- ids and sets --------------------*/

// each dtask has a unique id
typedef uint8_t dtask_id_t;

// sets of dtask ids
typedef uint32_t dtask_set_t;

// the maximum id that can fit in dtask_set_t
#define DTASK_MAX_ID (DTASK_BIT_WIDTH(dtask_set_t) - 1)

// all events with ids in x = (ID1 | ID2 ... | IDn) have succeeded
// can only be used inside a dtask
#define DTASK_AND(x) (!(~state->events & (x)))

// any event with an id in x = (ID1 | ID2 ... | IDn) has succeeded
// can only be used inside a dtask
#define DTASK_OR(x) (state->events & (x))

// convert an id to a singleton set (one bit)
static inline
dtask_set_t dtask_bit(dtask_id_t id) {
  return (1U << DTASK_MAX_ID) >> id;
}

/*-------------------- state and config --------------------*/

typedef struct dtask_state dtask_state_t;
typedef struct dtask dtask_t;
typedef struct dtask_config dtask_config_t;
typedef struct dtask_select dtask_select_t;

// describes a dtask and its relations
// a static const dtask table will be generated that can be stored in ROM
struct dtask
{
#ifndef NO_CLZ
  // updates state
  bool (*func)(dtask_state_t *state);
#endif
  // run when enabled
  void (*enable_func)(dtask_state_t *state);
  // run when disabled
  void (*disable_func)(dtask_state_t *state);
#ifndef NO_CLZ
  // set of direct dependents
  dtask_set_t dependents;
#endif
  // (direct or indirect) dependencies and dependents
  dtask_set_t all_dependencies, all_dependents;
};

// members present in all generated dtask state types
// events: set of successful (i.e. returns true) dtask ids
// config & select: see below
#define DTASK_STATE_HEADER              \
  struct                                \
  {                                     \
    dtask_set_t events;                 \
    const dtask_config_t config;        \
    dtask_select_t select;              \
  }

// stores const static config data
struct dtask_config
{
  // dtask table
  const dtask_t *tasks;
  // parent state
  const dtask_state_t *parent;
#ifdef NO_CLZ
  // static dtask_run() function
  dtask_set_t (*const run)(dtask_state_t *state, dtask_set_t initial);
#endif
  // graph state id
  const unsigned int id;
};

// dtask selection state
struct dtask_select
{
  dtask_set_t
    enabled,  // explicitly enabled
    disabled, // explicitly disabled
    selected; // dtask will only run if selected
  bool dirty; // selection needs to be updated
};

// base state type
// the script will generate an extended version of this type for each group
struct dtask_state
{
  DTASK_STATE_HEADER;
};

// update state, running dtasks in the initial set
dtask_set_t dtask_run(dtask_state_t *state, dtask_set_t initial);

// request to enable dtasks in the set
void dtask_enable(dtask_state_t *state, dtask_set_t set);

// request to disable dtasks in the set
void dtask_disable(dtask_state_t *state, dtask_set_t set);

// clear requests for dtasks in the set, i.e. enable only if needed
void dtask_clear(dtask_state_t *state, dtask_set_t set);

// explicitly enable dtasks in set, clear others
void dtask_switch(dtask_state_t *state, dtask_set_t set);

// update selection state
// requests from dtask_enable/disable/clear/switch only take effect after calling this
void dtask_select(dtask_state_t *state);


/*-------------------- dtask definition macros --------------------*/

// disable macros when preprocessing for script
#ifndef DTASK_GEN

// define a dtask
#define DTASK(name, ...)  \
  bool __dtask_##name(dtask_state_t *state)

// define a group
#define DTASK_GROUP(group_name) \
typedef group_name##_state_t dref_t;

// get a reference (pointer) to a dependency
// can only be used inside a dtask
#define DREF(x) ((x##_t *)&((dref_t *)state)->x)

// weak version of DREF that doesn't force dependency to be enabled
#define DREF_WEAK(x) DREF(x)

// define the enable function for a dtask
#define DTASK_ENABLE(name)  \
  void __dtask_enable_##name(dtask_state_t *state)

// define the disable function for a dtask
#define DTASK_DISABLE(name)  \
  void __dtask_disable_##name(dtask_state_t *state)

#endif

// convenient macros to create config/state with required const static members
#ifndef NO_CLZ
#define DTASK_CONFIG(name, parent, id) {(name), (dtask_state_t *)(parent), (id)}
#else
#define DTASK_CONFIG(name, parent, id) {(name), (dtask_state_t *)(parent), name##_run, (id)}
#endif

#define DTASK_STATE(name, parent, id) {{ .config = DTASK_CONFIG(name, parent, id) }}

/*-------------------- used by generated headers --------------------*/

// declare a dtask
#define DECLARE_DTASK(name, type...) \
  typedef type name##_t;             \
  bool __dtask_##name(dtask_state_t *state)

// declare enable function for a dtask
#define DECLARE_DTASK_ENABLE(name) \
  void __dtask_enable_##name(dtask_state_t *state)

// declare disable function for a dtask
#define DECLARE_DTASK_DISABLE(name) \
  void __dtask_disable_##name(dtask_state_t *state)

// default function for enable/disable
void __dtask_noop(dtask_state_t *state);

#endif
