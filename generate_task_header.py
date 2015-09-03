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
dget_re = re.compile(r'DREF(_WEAK)?\(\s*(\w+)\s*\)')
dtask_enable_or_disable_re = re.compile(r'DTASK_(EN|DIS)ABLE\(\s*(\w+)\s*\)')
dtask_group_re = re.compile(r'DTASK_GROUP\(\s*(\w+)\s*\)')


def init_task(tasks, name):
    if not name in tasks:
        tasks[name] = {
            'type': 'int',
            'deps': set(),
            'weak_deps': set(),
            'depnts': set(),
            'all_deps': set(),
            'all_depnts': set(),
            'en': False,
            'dis': False
        }


def find_tasks_in_file(filename):
    in_group = False
    tasks = {}

    last_task = None
    includes = ['-I' + i for i in options.include_dirs]
    macros = ['-D' + d for d in options.macros]
    cpp = subprocess.Popen(['cpp', '-DDTASK_GEN'] +
                           includes + macros + [filename],
                           stdout=subprocess.PIPE)
    lines = iter(cpp.stdout.readline, '')
    for line in lines:
        if line[0] != '#':

            #match DTASK_GROUP(...) definitions
            match = dtask_group_re.search(line)
            if match:
                group = match.group(1)
                in_group = group == options.target
                last_task = None

            if in_group:

                #match DTASK(...) definitions
                match = dtask_re.search(line)
                if match:
                    name = match.group(1)
                    type = match.group(2)
                    init_task(tasks, name)
                    tasks[name]['type'] = type
                    last_task = tasks[name]

                #match DTASK_(EN|DIS)ABLE(...) definitions
                match = dtask_enable_or_disable_re.search(line)
                if match:
                    name = match.group(2)
                    if match.group(1) == 'EN':
                        init_task(tasks, name)
                        tasks[name]['en'] = True
                    elif match.group(1) == 'DIS':
                        init_task(tasks, name)
                        tasks[name]['dis'] = True

                #match DREF(...) expressions
                for match in dget_re.finditer(line):
                    if match:
                        if match.group(1):
                            # weak dependency
                            last_task['weak_deps'].add(match.group(2))
                        else:
                            last_task['deps'].add(match.group(2))
    return tasks


def order_tasks(tasks):
    sorted = toposort_flatten({k: v['deps'] | v['weak_deps']
                               for (k, v) in tasks.iteritems()})

    #calculate dependents
    for name in sorted:
        for d in tasks[name]['deps']:
            tasks[d]['depnts'].add(name)
        for d in tasks[name]['weak_deps']:
            tasks[d]['depnts'].add(name)

    #calculate dependencies transitively
    for name in sorted:
        d = copy.deepcopy(tasks[name]['deps'])
        for dep in tasks[name]['deps']:
            d |= tasks[dep]['all_deps']
        tasks[name]['all_deps'] = d

    #calculate dependents transitively
    for name in sorted:
        for d in tasks[name]['all_deps']:
            tasks[d]['all_depnts'].add(name)

    return map(lambda name: (name, tasks[name]), sorted)


def show_set(s):
    return ' | '.join(
        map(lambda x: x.upper(), s)) if s else "0"


def dtask_bit(id):
    return (1 << (BITS_IN_DTASK_SET_T - 1)) >> id


def func_name(task_name, type, present):
    if present:
        return '__dtask_' + type + '_' + task_name
    else:
        return '__dtask_noop'


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

    tasks = {}
    for filename in sorted(files):
        new_tasks = find_tasks_in_file(filename)
        tasks.update(new_tasks)
    tasks = order_tasks(tasks)
    ids = {}
    id = 0

    with open(header, 'w') as f:
        f.write('''#ifndef __{name}__
#define __{name}__

#include "dtask.h"

'''.format(name=name.upper()))
        # id masks
        for (task, _) in tasks:
            f.write('#define {} 0x{:x}\n'.format(task.upper(), dtask_bit(id)))
            f.write('#define {}_ID {:d}\n'.format(task.upper(), id))
            ids[task] = id
            id = id + 1
        f.write('#define {}_COUNT {:d}\n'.format(name.upper(), id))

        f.write('\n#define {}_TASK_NAMES {{ \\\n'.format(name.upper()))
        for (task, _) in tasks:
            f.write('  "{}", \\\n'.format(task))
        f.write('}\n')

        #initial
        initial = set()
        for (task, dict) in tasks:
            if not dict['deps']:
                initial.add(task)

        f.write('\n#define {}_INITIAL ({})\n\n'.format(name.upper(),
                                                       show_set(initial)))

        #declare type
        f.write('typedef struct {}_data_s {{\n'.format(name))
        for (task, dict) in tasks:
            f.write('  {} {};\n'.format(dict['type'], task))
        f.write('}} {}_data_t;\n\n'.format(name))

        #declare tasks
        for (task, dict) in tasks:
            f.write('DECLARE_DTASK({}, {});\n'.format(task, dict['type']))
            if dict['en']:
                f.write('DECLARE_DTASK_ENABLE({});\n'.format(task))
            if dict['dis']:
                f.write('DECLARE_DTASK_DISABLE({});\n'.format(task))

        #dtask array
        f.write('''
static const dtask_t {}[{}] = {{
'''.format(name, id))
        for (task, dict) in tasks:
            f.write('''  {{
#ifndef NO_CLZ
    /* .func = */ __dtask_{task},
#endif
    /* .enable_func = */ {en},
    /* .disable_func = */ {dis},
#ifndef NO_CLZ
    /* .dependents = */ {depnts},
#endif
    /* .all_dependencies = */ {all_deps},
    /* .all_dependents = */ {all_depnts}
    }},\n'''.format(task=task,
                    en=func_name(task, 'enable', dict['en']),
                    dis=func_name(task, 'disable', dict['dis']),
                    depnts=show_set(dict['depnts']),
                    all_deps=show_set(dict['all_deps']),
                    all_depnts=show_set(dict['all_depnts'])))
        f.write(' };\n')

        #define the runner
        f.write('\n#ifdef NO_CLZ\n')

        #prologue
        f.write('''
#pragma GCC diagnostic ignored "-Wunused-function"
static dtask_set_t {name}_run(const dtask_state_t *state, void *data, dtask_set_t initial, dtask_set_t parent_events) {{
  const dtask_set_t selected = state->selected;
  dtask_set_t
    scheduled = initial & selected,
    events = 0;
'''.format(name=name))

        #dispatch code
        for (task, dict) in tasks:
            f.write('''
    if(({uptask} & scheduled) && __dtask_{task}(data, events, parent_events)) {{
    events |= {uptask};'''.format(task=task,
                                  uptask=task.upper()))
            if dict['depnts']:
                f.write('''
    scheduled |= ({depnts}) & selected;'''
                        .format(depnts=show_set(dict['depnts'])))

            f.write('''
  }
''')

        #epilogue
        f.write('''
  return events;
}
''')
        f.write('\n#endif\n')

        f.write('''
#ifndef DREF
#define DREF(x) ((x##_t *)&(({}_data_t *)data)->x)
#define DREF_WEAK(x) DREF(x)
#endif
'''.format(name))

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
