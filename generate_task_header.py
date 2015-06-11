#!/usr/bin/python

import os
import re
from toposort import toposort_flatten
import copy
import subprocess
import sys

BITS_IN_DTASK_SET_T = 32

dtask_re = re.compile(r'DTASK\(\s*(\w+)\s*,\s*(.+)\s*\)')
dget_re = re.compile(r'DGET\(\s*(\w+)\s*\)')


def find_tasks_in_file(filename):
    tasks = []
    cpp = subprocess.Popen(['cpp', '-w', filename],
                           stdout=subprocess.PIPE)
    lines = iter(cpp.stdout.readline, '')
    for line in lines:
        if line[0] != '#':
            match = dtask_re.search(line)
            if match:
                #print(match.group(0))
                tasks.append({'name': match.group(1),
                              'type': match.group(2),
                              'deps': set()})
            for match in dget_re.finditer(line):
                if match:
                    #print(match.group(0))
                    tasks[-1]['deps'].add(match.group(1))
    return tasks


def order_tasks(tasks):
    types = {}
    deps = {}
    depnts = {}
    all_deps = {}
    all_depnts = {}

    #build dictionaries
    for task in tasks:
        types[task['name']] = task['type']
        deps[task['name']] = task['deps']

    sorted = toposort_flatten(copy.deepcopy(deps))

    #calculate dependents
    for name in sorted:
        depnts[name] = set()
        for d in deps[name]:
            depnts[d].add(name)

    #calculate dependencies transitively
    for name in sorted:
        d = copy.deepcopy(deps[name])
        all_deps[name] = d
        for dep in deps[name]:
            d |= all_deps[dep]
        all_deps[name] = d

    #calculate dependents transitively
    for name in sorted:
        all_depnts[name] = set()
        for d in all_deps[name]:
            all_depnts[d].add(name)

    return map(lambda name: (name, types[name],
                             deps[name],
                             depnts[name],
                             all_deps[name],
                             all_depnts[name]),
               sorted)


def show_set(s):
    return ' | '.join(
        map(lambda x: x.upper(), s)) if s else "0"


def dtask_bit(id):
    return (1 << (BITS_IN_DTASK_SET_T - 1)) >> id

def generate_header(name, files):
    #touch the header file
    header = name + '.h'
    with open(header, 'w') as f:
        os.utime(header, None)
        f.write('#undef DTASK\n')
        f.write('#undef DGET\n')

    tasks = []
    for filename in sorted(files):
        new_tasks = find_tasks_in_file(filename)
        tasks.extend(new_tasks)
    tasks = order_tasks(tasks)
    ids = {}
    id = 0

    with open(header, 'w') as f:
        f.write('''#ifndef __{name}__
#define __{name}__

#include "dtask.h"

'''.format(name=name.upper()))
        # id masks
        for (task, _, _, _, _, _) in tasks:
            f.write('#define {} 0x{:x}\n'.format(task.upper(), dtask_bit(id)))
            ids[task] = id
            id = id + 1

        #initial
        initial = set()
        for (task, _, deps, _, _, _) in tasks:
            if not deps:
                initial.add(task)

        f.write('\n#define {}_INITIAL ({})\n\n'.format(name.upper(),
                                                       show_set(initial)))

        #declare tasks
        for (task, type, _, _, _, _) in tasks:
            f.write('DECLARE_DTASK({}, {});\n'.format(task, type))

        #dtask array
        f.write('\nstatic const dtask_t {}[{}] = {{ \\\n'.format(name, id))
        for (task, type, deps, depnts, all_deps, all_depnts) in tasks:
            f.write('''  {{ /* .all_dependencies = */ {}, \\
    /* .all_dependents = */ {} \\
    }}, \\\n'''.format(show_set(all_deps),
                       show_set(all_depnts)))
        f.write(' };\n')

        #define the runner

        #prologue
        f.write('''
static void {name}_run(const dtask_state_t *state, dtask_set_t initial) {{
  const dtask_set_t enabled = state->enabled_dependencies;
  dtask_set_t
    scheduled = initial & enabled,
    events = 0;

#ifndef NO_CLZ
  static const void *dispatch_table[] = {{
'''.format(name=name))

        #dispatch table
        for (task, _, _, _, _, _) in tasks:
            f.write('    &&__{},\n'.format(task))
        f.write('''  };
#endif
''')

        #dispatch code
        for (task, _, _, depnts, _, _) in tasks:
            f.write('''
  if(!scheduled) goto end;
#ifndef NO_CLZ
  goto *dispatch_table[__builtin_clz(scheduled)];
__{task}:
#endif
  if(({uptask} & scheduled) && __dtask_{task}(events)) {{
    events |= {uptask};
    scheduled |= ({depnts}) & enabled;
  }}
            scheduled &= ~{uptask};
#ifdef {upname}_AFTER_TASK_HOOK
   {upname}_AFTER_TASK_HOOK;
#endif
'''
                    .format(task=task,
                            uptask=task.upper(),
                            depnts=show_set(depnts),
                            upname=name.upper()))

        #epilogue
        f.write('''
end:
  return;
}
''')

        f.write('''
#endif
''')

def main(argv):
    generate_header(argv[1], argv[2:])

if __name__ == "__main__":
    main(sys.argv)
