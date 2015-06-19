#!/usr/bin/python

import os
import re
from toposort import toposort_flatten
import copy
import subprocess
import argparse

BITS_IN_DTASK_SET_T = 32
options = None
dtask_re = re.compile(r'DTASK\(\s*(\w+)\s*,\s*(.+)\s*\)')
dget_re = re.compile(r'DGET\(\s*(\w+)\s*\)')


def find_tasks_in_file(filename):
    tasks = []
    includes = ['-I' + i for i in options.include_dirs]
    macros = ['-D' + d for d in options.macros]
    cpp = subprocess.Popen(['cpp'] + includes + macros + [filename],
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


def generate_header():
    name = options.target
    files = options.source
    #touch the header file
    if(options.output_file):
        header = options.output_file
    else:
        header = name + '.h'

    with open(header, 'w') as f:
        os.utime(header, None)
        f.write('#undef DTASK\n')
        f.write('#undef DGET\n')
        f.flush()

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
        f.write('''
static const dtask_t {}[{}] = {{
'''.format(name, id))
        for (task, type, deps, depnts, all_deps, all_depnts) in tasks:
            f.write('''  {{
#ifndef NO_CLZ
    /* .task = */ __dtask_{task},
    /* .dependencies = */ {depnts},
#endif
    /* .all_dependencies = */ {all_deps},
    /* .all_dependents = */ {all_depnts}
    }},\n'''.format(task=task,
                    depnts=show_set(depnts),
                    all_deps=show_set(all_deps),
                    all_depnts=show_set(all_depnts)))
        f.write(' };\n')

        #define the runner
        f.write('\n#ifdef NO_CLZ\n')

        #prologue
        f.write('''
#pragma GCC diagnostic ignored "-Wunused-function"
static void {name}_run(const dtask_state_t *state, dtask_set_t initial) {{
  const dtask_set_t enabled = state->enabled_dependencies;
  dtask_set_t
    scheduled = initial & enabled,
    events = 0;
'''.format(name=name))

        #dispatch code
        for (task, _, _, depnts, _, _) in tasks:
            f.write('''
  if(({uptask} & scheduled) && __dtask_{task}(events)) {{
    events |= {uptask};
    scheduled |= ({depnts}) & enabled;
  }}
'''
                    .format(task=task,
                            uptask=task.upper(),
                            depnts=show_set(depnts),
                            upname=name.upper()))

        #epilogue
        f.write('''
  return;
}
''')
        f.write('\n#endif\n')

        f.write('''
#endif
''')


def main():
    global options
    parser = argparse.ArgumentParser(description='Generate a DTask header')
    parser.add_argument('source', nargs='*',
                        help='a source file to process')
    parser.add_argument('--target', dest='target', help='target name',
                        required=True)
    parser.add_argument('-o', dest='output_file', help='output file')
    parser.add_argument('-I', dest='include_dirs', metavar='DIR',
                        action='append', help='include dir', default=[])
    parser.add_argument('-D', dest='macros', metavar='MACRO',
                        action='append', help='define macro', default=[])
    options, _ = parser.parse_known_args()
    if(options.source):
        generate_header()

if __name__ == "__main__":
    main()
