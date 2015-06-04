#!/usr/bin/python

import os
import re
from toposort import toposort_flatten
import copy
import subprocess

dtask_re = re.compile(r'DTASK\(\s*(\w+)\s*,(.+)\)')
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
                              'type': match.group(2).strip(),
                              'deps': set()})
            for match in dget_re.finditer(line):
                if match:
                    #print(match.group(0))
                    tasks[-1]['deps'].add(match.group(1))
    return tasks


def find_tasks(dir):
    tasks = []
    for root, dirs, files in os.walk(dir):
        for filename in files:
            ext = os.path.splitext(filename)[1][1:]
            if ext == 'c' or ext == 'cpp':
                new_tasks = find_tasks_in_file(os.path.join(root, filename))
                tasks.extend(new_tasks)
    return tasks


def order_tasks(tasks):
    types = {}
    deps = {}
    for task in tasks:
        types[task['name']] = task['type']
        deps[task['name']] = task['deps']
    deps_copy = copy.deepcopy(deps)
    return map(lambda name: (name, types[name], deps_copy[name]),
               toposort_flatten(deps))


def generate_header(dir, header):
    #touch the header file
    with open(header, 'w') as f:
        os.utime(header, None)
        f.write('#undef DTASK\n')
        f.write('#undef DGET\n')
    tasks = order_tasks(find_tasks(dir))
    ids = {}
    id = 0

    with open(header, 'w') as f:
        f.write('''#ifndef __ALL_TASKS__
#define __ALL_TASKS__

#include "dtask.h"

''')
        for (task, type, deps) in tasks:
            f.write('#define {} 0x{:x}\n'.format(task.upper(), 1 << id))
            ids[task] = id
            id = id + 1
        f.write('\n')
        f.write('#define ALL_TASKS { \\\n')
        for (task, type, deps) in tasks:
            f.write('  {{ __dtask_{}, "{}", {}, {:d} }}, \\\n'
                    .format(task,
                            task, ' | '.join(map(lambda x: x.upper(), deps)),
                            ids[task]))
        f.write(' }\n\n')
        for (task, type, deps) in tasks:
            f.write('DECLARE_DTASK({}, {});\n'.format(task, type))
        f.write('''
#endif
''')

generate_header('.', 'all_tasks.h')
