DTask
=====

DTask is a scheduler for statically dependent tasks.


Many embedded sensor applications can be structured as a set of dependent computations starting from an interrupt. Intermediate computations may be shared, such as filtered data. This is a type of data flow programming. DTask is designed to make this easy to express, and yet require minimal resources.  DTask could replace an RTOS in some situations, and is much less complicated.

DTask uses annotations in the source code to calculate task dependencies during compilation, and then sorts them topologically. At runtime, the task are processed in waves, so that results can be passed atomically without locking. Tasks can be enabled and disabled at runtime, which will automatically enable/disable other related tasks.

DTask allows definition of many small modular tasks that will be easier to test, and yet can be inlined for performance.

Usage
=====

Declare a task like this:

```
DTASK(task_name, result_type)
{
  bool z_is_valid;
  result_type z;

  int x = *DREF(an_integer);
  int y = *DREF(another_integer);
  // compute z from x and y, set z_is_valid if the computation yields a (new/valid) result
  if(z_is_valid) {
    task_name = z;
  }
  return z_is_valid;
}
```

Code can be added to run when the task is enabled or disabled:

```
DTASK_ENABLE(task_name)
{
  // run when task_name is enabled
}

DTASK_DISABLE(task_name)
{
  // run when task_name is disabled
}
```

Tasks are declared in groups. A task group is declared with:

```
DTASK_GROUP(group_name)
```

The following tasks are part of this group.  This will create a struct type that contains the state named `group_name_state`, which contains a field for every task.

API
===

Outside of a task (or from another task group, see below):

- `dtask_enable(state, TASK1 | TASK2)`: Enable TASK1 and TASK2 (and dependencies)
- `dtask_disable(state, TASK1 | TASK2)`: Disable TASK1 and TASK2 (and dependents)
- `dtask_clear(state, TASK1 | TASK2)`: Only enable TASK1 and TASK2 if enabled tasks depend on them
- `dtask_switch(state, TASK1 | TASK2)`: Enable only TASK1 and TASK2 and depencencies, disable all others
- `dtask_select(state)`: commit the changes from the above calls, and run DTASK_ENABLE/DTASK_DISABLE code for tasks that are enabled/disabled.
- `dtask_run(state, INITIAL_TASK)`: run tasks, running the task named `initial` unconditionally.

And within a task:

- to get a value: `x = *DREF(task_name);`
- to set the result of a task: `*DREF(task_name) = x;`
    - `task_name` should be the name of the current task only.

See `main.c`, and `*_tasks.c` for an example. Run `make && ./test` to build and run the example.

Delays
======

Delays can be used to look at the history of a value, useful to implement FIR filters for example.

Use `DELAY(type, length)` as a task type to implement a delay.  Then use the following API:

- `DELAY_READ(delay, type, len, i)`: returns a pointer to the value `i` waves ago
- `DELAY_WRITE(delay, type, len, value)`: writes the value for the current wave
- `DELAY_FILL(delay, type, len, value)`: fills the delay with a value

The delay must be declared with `DECLARE_DELAY(type, length)`.

See `factor_tasks.c` for an example.

Nesting
=======

Task groups can be nested by calling `dtask_run()` from a task in another task group.

See `toplevel_tasks.c` for an example.

