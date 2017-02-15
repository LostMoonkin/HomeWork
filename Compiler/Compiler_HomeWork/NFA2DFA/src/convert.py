#!/usr/bin/env python3
"""
Compiler HomeWork:
NFA table to DFA table
"""
import json
import sys
from collections import deque


def wrong_input():
    print(
        'Use ./convert.py -i {nfa json file name} -o {dfa dump json file name} -t {test string file}')
    sys.exit(-1)


def load_NFA_json(nfa_file):

    try:
        with open(nfa_file, 'r') as f:
            nfa_json = json.load(f)
            return (set(nfa_json["k"]), set(nfa_json["e"]), nfa_json[
                    "f"], set(nfa_json["s"]), set(nfa_json["z"]))
    except:
        wrong_input()


def set_cache(edge):
    cache = {'#': {}}

    for e in edge:
        cache[e] = {}

    return cache


def closure(f, cache, I, edge):
    res = set()

    for i in I:
        if i not in cache:
            cache[i] = set()
            if edge == '#':
                cache[i].add(i)
            if i in f:
                if edge in f[i]:
                    if edge == '#':
                        cache[i] |= closure(f, cache, set(f[i][edge]), '#')
                    else:
                        cache[i] = set(f[i][edge])

        res |= cache[i]
    return res


def e_closure(f, cache, I):

    return closure(f, cache['#'], I, '#')


def move(f, cache, I, edge):

    return closure(f, cache[edge], I, edge)


def set_default_dfa(nfa_e):

    return {'k': [], 'e': list(nfa_e), 'f': {}, 's': [], 'z': []}


def get_dfa(nfa_k, nfa_e, nfa_f, nfa_s, nfa_z):

    dfa = set_default_dfa(nfa_e)

    dfa_set = []

    cache = set_cache(nfa_e)

    ep = e_closure(nfa_f, cache, nfa_s)

    queue = deque([ep])

    dfa_set.append([ep])

    dfa['k'].append('0')
    dfa['s'].append('0')

    if len(ep & nfa_z):
        dfa['z'].append('0')

    i = 0
    while queue:

        T = queue.popleft()
        j = ''
        index = str(i)
        i += 1
        dfa["f"][index] = {}

        for s in nfa_e:

            t = e_closure(nfa_f, cache, move(nfa_f, cache, T, s))
            try:
                j = str(dfa_set.index(t))
            except ValueError:
                queue.append(t)
                j = str(len(dfa_set))
                dfa_set.append(t)
                dfa["k"].append(j)
            dfa["f"][index][s] = j
            if len(t & nfa_s):
                dfa["s"].append(j)
            if len(t & nfa_z):
                dfa["z"].append(j)

    return dfa


def dfa_dump(dfa, dfa_file):

    with open(dfa_file, 'w') as f:
        json.dump(dfa, f, indent=4, sort_keys=True)


def parse_string(dfa, string):

    pos = dfa['s'][0]

    for s in string:
        if s not in dfa['e']:
            return False

        try:
            pos = dfa['f'][pos][s]
        except:
            return False

    if pos in dfa['z']:
        return True

    return False


if __name__ == '__main__':

    args = sys.argv

    if args[1] != '-i' or args[3] != '-o':
        wrong_input()

    (nfa_k, nfa_e, nfa_f, nfa_s, nfa_z) = load_NFA_json(args[2])

    dfa = get_dfa(nfa_k, nfa_e, nfa_f, nfa_s, nfa_z)

    dfa_dump(dfa, args[4])

    if len(args) < 6:
        sys.exit(0)

    if args[5] != '-t':
        wrong_input()

    try:
        with open(args[6], 'r') as f:
            string = f.readline()
            string = string.replace('\n', '')
            strings = []
            while string:
                string = string.replace('\n', '')
                strings.append(string)
                string = f.readline()
    except:
        wrong_input()

    for s in strings:
        res = 'passed' if parse_string(dfa, s) else 'failed'
        print('[' + s + ']' + ' ' + res)
