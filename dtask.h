#ifndef __DTASK__
#define __DTASK__

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t dtask_mask_t;
typedef uint8_t dtask_id_t;

typedef struct dtask dtask_t;
struct dtask
{
  bool (*func)(const dtask_t *, uint32_t);
  char *name;
  dtask_mask_t dependencies, dependents, all_dependencies, all_dependents;
  dtask_id_t id;
};

typedef struct dtask_state
{
  const dtask_t *tasks;
  const dtask_id_t num_tasks;
  dtask_mask_t enabled;
} dtask_state_t;

void dtask_run(dtask_state_t *state, dtask_mask_t initial);
void dtask_enable(dtask_state_t *state, dtask_mask_t mask);
void dtask_disable(dtask_state_t *state, dtask_mask_t mask);
void dtask_disable_all(dtask_state_t *state);

#define DGET(x) (x)

#define DTASK(name, ...)  \
  name##_t name;          \
  bool __dtask_##name(const dtask_t *task, dtask_mask_t events)

#define DECLARE_DTASK(name, type...)  \
  typedef type name##_t;             \
  extern name##_t name;              \
  bool __dtask_##name(const dtask_t *task, dtask_mask_t events)

#include "all_tasks.h"

#define DTASK_AND(x) (!(~events & (x)))
#define DTASK_OR(x) (events & (x))

#endif
