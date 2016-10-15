#!/usr/bin/python

# Copyright 2016 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import re
from third_party.toposort import toposort_flatten
import copy
import subprocess
import argparse

# options read from argparse
options = None

# regexs

# DTASK(name, type)
dtask_re = re.compile(r'DTASK\(\s*(\w+)\s*,\s*(.+)\s*\)')
# DGET(name)
dget_re = re.compile(r'DREF(_WEAK)?\(\s*(\w+)\s*\)')
# DTASK_ENABLE(name) | DTASK_DISABLE(name)
dtask_enable_or_disable_re = re.compile(r'DTASK_(EN|DIS)ABLE\(\s*(\w+)\s*\)')
# DTASK_GROUP(group_name)
dtask_group_re = re.compile(r'DTASK_GROUP\(\s*(\w+)\s*\)')

# initialize task object with name if not present in tasks
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

# look through file line by line, gathering data from regex matches
def find_tasks_in_file(filename):
    in_group = False
    in_dtask = False
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
                in_dtask = False

            if in_group:

                #match DTASK(...) definitions
                match = dtask_re.search(line)
                if match:
                    name = match.group(1)
                    type = match.group(2)
                    init_task(tasks, name)
                    tasks[name]['type'] = type
                    last_task = tasks[name]
                    in_dtask = True

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
                    in_dtask = False

                #match DREF(...) expressions
                if in_dtask:
                    for match in dget_re.finditer(line):
                        if match:
                            if match.group(1):
                                # weak dependency
                                last_task['weak_deps'].add(match.group(2))
                            else:
                                last_task['deps'].add(match.group(2))
    return tasks

# toposort, then calculate dependencies
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

# generate bit set expression in A | B | C format
def show_set(s):
    return ' | '.join(
        map(lambda x: x.upper(), s)) if s else "0"


# id to bit set conversion
def dtask_bit(id):
    return (1 << (options.bits - 1)) >> id

# generate a function name from the task name
def func_name(task_name, type, present):
    if present:
        return '__dtask_' + type + '_' + task_name
    else:
        return '__dtask_noop'

# generate a header based on the script arguments
def generate_header():
    name = options.target
    files = options.source

    # touch the header file so that #include "header" doesn't cause cpp to fail
    if(options.output_file):
        header = options.output_file
    else:
        header = name + '.h'

    with open(header, 'w') as f:
        os.utime(header, None)

    # build information from the files
    tasks = {}
    for filename in sorted(files):
        new_tasks = find_tasks_in_file(filename)
        tasks.update(new_tasks)
    tasks = order_tasks(tasks)
    ids = {}
    id = 0

    # preamble
    with open(header, 'w') as f:
        f.write('''#ifndef __{name}__
#define __{name}__

#include "dtask.h"

'''.format(name=name.upper()))

        # define id macros
        for (task, _) in tasks:
            f.write('#define {} 0x{:x}\n'.format(task.upper(), dtask_bit(id)))
            f.write('#define {}_ID {:d}\n'.format(task.upper(), id))
            ids[task] = id
            id = id + 1
        f.write('#define {}_COUNT {:d}\n'.format(name.upper(), id))

        # define a name string table
        f.write('\n#define {}_TASK_NAMES {{ \\\n'.format(name.upper()))
        for (task, _) in tasks:
            f.write('  "{}", \\\n'.format(task))
        f.write('}\n')

        # initial tasks, can only be run when flagged as initial tasks
        initial = set()
        for (task, dict) in tasks:
            if not dict['deps']:
                initial.add(task)

        f.write('\n#define {}_INITIAL ({})\n\n'.format(name.upper(),
                                                       show_set(initial)))

        # declare state type
        f.write('typedef struct {}_state {{\n'.format(name))
        f.write('  DTASK_STATE_HEADER;\n')

        for (task, dict) in tasks:
            f.write('  {} {};\n'.format(dict['type'], task))
        f.write('}} {}_state_t;\n\n'.format(name))

        #declare task functions
        for (task, dict) in tasks:
            f.write('DECLARE_DTASK({}, {});\n'.format(task, dict['type']))
            if dict['en']:
                f.write('DECLARE_DTASK_ENABLE({});\n'.format(task))
            if dict['dis']:
                f.write('DECLARE_DTASK_DISABLE({});\n'.format(task))

        # dtask table
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

        # define the NO_CLZ run function (compare to dtask_run)
        f.write('\n#ifdef NO_CLZ\n')

        # prologue
        f.write('''
#pragma GCC diagnostic ignored "-Wunused-function"
static dtask_set_t {name}_run(dtask_state_t *state, dtask_set_t initial) {{
  const dtask_set_t selected = state->select.selected;
  dtask_set_t
    scheduled = initial & selected;
  state->events = 0;
'''.format(name=name))

        # dispatch code
        for (task, dict) in tasks:
            f.write('''
  if(({uptask} & scheduled) && __dtask_{task}(state)) {{
    state->events |= {uptask};'''.format(task=task,
                                         uptask=task.upper()))
            if dict['depnts']:
                f.write('''
    scheduled |= ({depnts}) & selected;'''
                        .format(depnts=show_set(dict['depnts'])))

            f.write('''
  }
''')

        # epilogue
        f.write('''
  return state->events;
}
''')

        f.write('''
#endif
#endif
''')


def main():

    # run argparse
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
    parser.add_argument('-b', dest='bits', type=int, default=32, help='bit width of dtask_set_t')
    options, _ = parser.parse_known_args()

    # generate the header
    if(options.source):
        generate_header()

if __name__ == "__main__":
    main()
