"""
Created by Yemeibeining

11.2.2016
"""

import pickle
import sys
from math import log2


class Node:
    # Decision id Node

    def __init__(self, parent=None, data_set=None):
        self.data_set = data_set
        self.result = parent
        self.attr = None
        self.parent = parent
        self.child = {}


def entropy(props):
    # 计算信息熵

    if not isinstance(props, (list, tuple)):
        return None

    e = 0.0
    for p in props:
        e -= p * log2(p)

    return e


def info_gain(data_set, axis, element_id=-1, return_ratio=False):
    # 计算特征 axis 对数据集对信息增益g(d, a)

    if not isinstance(data_set, (list, tuple)) or not isinstance(axis, int):
        return None

    dic = {}
    da = {}
    cda = {}

    for t in data_set:
        dic[t[element_id]] = dic.get(t[element_id], 0) + 1
        da[t[axis]] = da.get(t[axis], 0) + 1
        cda[(t[element_id], t[axis])] = cda.get(
            (t[element_id], t[axis]), 0) + 1

    pc = map(lambda x: x / len(data_set), dic.values())
    entropy_data_set = entropy(tuple(pc))

    pcd_axis = {}
    for key, value in cda.items():
        axis = key[1]
        pca = value / da[axis]
        pcd_axis.setdefault(axis, []).append(pca)

    condition_entropy = 0.0
    for a, v in da.items():
        p = v / len(data_set)
        e = entropy(pcd_axis[a])
        condition_entropy += e * p

    if return_ratio:
        return (entropy_data_set - condition_entropy) / entropy_data_set
    else:
        return entropy_data_set - condition_entropy


def get_result(data_set, element_id=-1):
    # 获取data_set中实例数最大的目标特征值

    if not isinstance(
        data_set, (tuple, list)) or not isinstance(
            element_id, int):
        return None

    count = {}
    for t in data_set:
        count[t[element_id]] = count.get(t[element_id], 0) + 1

    max_count, result = 0, None
    for key, value in count.items():
        if value > max_count:
            max_count = value
            result = key

    return result


def devide_set(data_set, axis):
    # 根据特征值a分裂data_set

    if not isinstance(data_set, (tuple, list)) or not isinstance(axis, int):
        return None

    subset = {}

    for t in data_set:
        subset.setdefault(t[axis], []).append(t)

    return subset


def build_decision_tree(
        data_set,
        axis,
        threshold=0.0001,
        tree=None,
        element_id=-1):
    # 建立决策树

    if tree is not None and not isinstance(tree, Node):
        return None

    if not isinstance(data_set, (tuple, list)) or not isinstance(axis, set):
        return None
    if not tree:
        tree = Node(parent=None, data_set=data_set)

    subset = devide_set(data_set=data_set, axis=element_id)

    if len(subset) <= 1:
        for key in subset.keys():
            tree.result = key
        del subset
        return tree

    if len(axis) <= 0:
        tree.result = get_result(data_set=data_set)
        return tree

    use_gain_ratio = True
    max_gain = 0.0
    attr_id = -1

    for a in axis:
        gain = info_gain(
            data_set=data_set,
            axis=a,
            return_ratio=use_gain_ratio)

        if gain > max_gain:
            max_gain = gain
            attr_id = a

    if max_gain < threshold:
        tree.result = get_result(data_set=data_set)
        return tree

    tree.attr = attr_id
    sub_data_set = devide_set(data_set=data_set, axis=attr_id)
    del data_set[:]

    tree.data_set = None
    axis.discard(attr_id)

    for key in sub_data_set.keys():
        temp_tree = Node(parent=tree, data_set=sub_data_set.get(key))
        tree.child[key] = temp_tree
        build_decision_tree(
            data_set=sub_data_set.get(key),
            axis=axis,
            threshold=threshold,
            element_id=element_id,
            tree=temp_tree)

    return tree


def classify(tree, instance):
    if not tree:
        return None

    if tree.result:
        return tree.result

    return classify(tree=tree.child[instance[tree.attr]], instance=instance)


def get_next_ball(tree, red_ball):

    red_ball_next = [None for i in range(0, 7)]

    for i in range(1, 7):
        if i == 1:
            temp_instance = red_ball[2: 7]
            red_ball_next[i] = classify(tree=tree[i], instance=temp_instance)
        else:
            temp_instance = red_ball[1: i]
            temp_instance.extend(red_ball[i + 1: 7])
            red_ball_next[i] = classify(tree=tree[i], instance=temp_instance)

    check_ball = set(red_ball_next)

    if not len(check_ball) == len(red_ball_next):
        return get_next_ball(tree=tree, red_ball=red_ball_next)
    else:
        return red_ball_next


if __name__ == '__main__':

    argv = sys.argv

    if not len(argv) == 7:
        print(
            'Use TicketPrediction.py {red_1} {red_2} ... {red_6} to run the code.\n'
            'Like TicketPrediction.py 04 09 11 17 26 27')
        exit()

    red_ball = [int(argv[i]) for i in range(1, 7)]

    # for test
    # red_ball = [4, 9, 11, 17, 26, 27]

    with open('./data/DecisionTreeDataSet', 'rb') as f:
        data_set_all = pickle.load(f)

    tree = [None for i in range(0, 7)]

    print('Building decision tree...')

    for i in range(1, 7):

        data_set = data_set_all['red_' + str(i)]
        count = len(data_set[0]) - 1

        tree[i] = build_decision_tree(
            data_set=data_set,
            axis=set(range(0, count))
        )

    red_ball_next = get_next_ball(tree=tree, red_ball=red_ball)

    print('\nNext red balls:')
    print('%02d %02d %02d %02d %02d %02d' %
          (red_ball_next[1], red_ball_next[2], red_ball_next[3], red_ball_next[4], red_ball_next[5], red_ball_next[6]))
