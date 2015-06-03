#!/usr/bin/python

import os
import re
import sys
from toposort import toposort, toposort_flatten
import copy

dtask_re = re.compile('DTASK\(\s*(\w+)\s*,\s*(\w+)\s*\)')
dget_re = re.compile('DGET\(\s*(\w+)\s*\)')


def find_tasks_in_file(filename):
    tasks = []
    with open(filename) as f:
        for line in f:
            match = dtask_re.search(line)
            if match:
                tasks.append({'name': match.group(1),
                              'type': match.group(2),
                              'deps': set()})
            match = dget_re.search(line)
            if match:
                tasks[-1]['deps'].add(match.group(1))
    return tasks

#print(find_tasks_in_file('tasks.c'))

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
    return map(lambda name: (name, types[name], deps_copy[name]), toposort_flatten(deps))

def generate_header(dir, header):
    tasks = order_tasks(find_tasks(dir))
    ids = {}
    id = 0

    with open(header, 'w') as f:
        f.write('''#ifndef __ALL_TASKS__
#define __ALL_TASKS__
#include "dtask.h"

typedef enum task_id_t {
''')
        for (task, type, deps) in tasks:
            f.write('  {} = {},\n'.format(task.upper(), id))
            ids[task] = id
            id = id + 1
        f.write('  MAX_TASK_ID = {}\n'.format(id))
        f.write('} task_id_t;\n\n')
        f.write('#define ALL_TASKS { ')
        for (task, type, deps) in tasks:
            mask = 0
            for d in deps:
                mask = mask | (1 << ids[d])
            f.write('{{ {}, "{}", 0x{:x}, {:d} }}, '.format(task, task, mask, ids[task]))
        f.write(' }\n\n')
        for (task, type, deps) in tasks:
            f.write('DTASK({}, {});\n'.format(task, type))
        f.write('''
#endif
''')

generate_header('.', 'all_tasks.h')
