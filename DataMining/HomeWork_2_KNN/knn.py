"""
Created by Yemeibeining

11.16.2016
"""

import sys
import xlrd
import math
import random
import heapq


class Dmr:

    def __init__(self, label, data, index):
        self.label = label
        self.data = data
        self.index = index


def wrong_input():
    print('Wrong Input.')
    print(
        'Use python3 knn.py [-l] {label file} [-d] {data file(*.xls)} [dist func] to run the code.\n')
    print(
        '[dist func]:\n' +
        '            [-c] cosine\n' +
        '            [-a] adjust cosine\n' +
        '            [-p] pearson\n' +
        '            [-e] euclidean\n')
    sys.exit(-1)


def load_dmr(label_file, data_file):
    labels = []
    data = []

    try:
        with open(label_file, 'r') as f:
            label = f.readline()

            while label:
                labels.append(label.replace('\n', ''))
                label = f.readline()
    except:
        wrong_input()

    try:
        xls_data = xlrd.open_workbook(data_file)
        table = xls_data.sheet_by_index(0)
        for row in range(table.nrows):
            data.append(table.row_values(row))

    except:
        wrong_input()

    dmr_set = []
    dmr_attr_avg = []

    attr_maxs = []

    for i in range(len(data[0])):
        attr_sum = 0
        attr_max = 0
        for index in range(len(data)):
            attr_sum += data[index][i]
            attr_max = max(attr_max, data[index][i])
        dmr_attr_avg.append(attr_sum / len(data))
        attr_maxs.append(attr_max)

    #
    # for i in range(len(data[0])):
    #     for index in range(len(data)):
    #         data[index][i] /= attr_maxs[i]

    for i in range(len(data)):
        attr_max, attr_min = 0, 1000
        for j in range(len(data[0])):
            attr_max, attr_min = max(attr_max, data[i][j]), min(attr_min, data[i][j])
        for j in range(len(data[0])):
            data[i][j] = (data[i][j] - attr_min) / (attr_max - attr_min)

    for i in range(len(data)):
        dmr_set.append(Dmr(labels[i], data[i], i))

    return dmr_set, dmr_attr_avg


def get_dist_cosine(x, y, dmr_avg=None):

    sxx, syy, xy = sum([i * i for i in x]), \
        sum([i * i for i in y]), \
        sum([x[i] * y[i] for i in range(len(x))])

    return xy / (math.sqrt(sxx) * math.sqrt(syy))


def get_dist_adjust_cosine(x, y, avg=None):

    if avg:
        for i in range(len(x)):
            x[i] -= avg[i]
            y[i] -= avg[i]

        return get_dist_cosine(x, y)
    return get_dist_cosine(x, y)


def get_dist_pearson(x, y, dmr_avg=None):

    length = len(x)

    xy = [x[i] * y[i] for i in range(length)]
    xx, yy = [i * i for i in x], [i * i for i in y]

    avg_xy, avg_xx, avg_yy, avg_x, avg_y = sum(
        xy) / length, sum(xx) / length, sum(yy) / length, sum(x) / length, sum(y) / length

    return (avg_xy - avg_x * avg_y) / \
        (math.sqrt(avg_xx - avg_x ** 2) * math.sqrt(avg_yy - avg_y ** 2))


def get_dist_euclidean(x, y, dmr_avg=None):

    xy = [(x[i] - y[i]) ** 2 for i in range(len(x))]

    return math.sqrt(sum(xy))


def cross_validation(count, dist_func, k_num, dmr_set, dmr_avg=None):

    size = int(len(dmr_set) / count)
    visit = [0] * len(dmr_set)
    group = [[] for i in range(count)]
    flag = 0

    for i in range(count):
        if flag == len(dmr_set):
            break
        for j in range(size):
            if flag == len(dmr_set):
                break
            pos = random.randint(0, len(dmr_set) - 1)
            while visit[pos]:
                pos = random.randint(0, len(dmr_set) - 1)
            flag += 1
            group[i].append(dmr_set[pos])
            visit[pos] = 1

    passed, failed = 0, 0
    for i in range(count):
        test = group[i]
        train = list(set(dmr_set) - set(test))

        for data in test:
            label = knn(dist_func, data, train, k_num, dmr_avg)
            if label == data.label:
                passed += 1
            else:
                failed += 1

    return passed / (passed + failed)


def knn(dist_func, dmr_data, dmr_set, k_num, dmr_avg=None):

    k = []
    res = {}
    for dmr_temp in dmr_set:
        rate = dist_func(
            dmr_data.data, dmr_temp.data, dmr_avg)
        k.append((rate, dmr_temp.label, dmr_temp.index))
        if dmr_temp.label not in res:
            res[dmr_temp.label] = 0

    top_k = heapq.nlargest(k_num, k) if dist_func != get_dist_euclidean else heapq.nsmallest(k_num, k)
    for (rate, label, index) in top_k:
        res[label] += 1

    max_label = ''
    max_value = -1
    for label, value in res.items():
        if value > max_value:
            max_value, max_label = value, label

    return max_label


def loocv(dmr_data, dist_func, dmr_set, k_num, dmr_avg=None):

    dmr_set = list(set(dmr_set) - set([dmr_data]))

    label = knn(dist_func, dmr_data, dmr_set, k_num, dmr_avg)

    return label == dmr_data.label


def test_cross_count(cross_count, dist_func, k_num, dmr_set, dmr_avg=None):
    rate_all = 0
    for t in range(cross_count):
        rate = 0
        for i in range(cross_count):
            rate += cross_validation(cross_count,
                                     dist_func, k_num, dmr_set, dmr_avg)

        rate /= cross_count
        # print(rate)
        rate_all += rate
    rate_all /= cross_count

    return rate_all


if __name__ == '__main__':

    args = sys.argv
    label_file, data_file = '', ''

    if len(args) < 6:
        wrong_input()

    if args[1] == '-l':
        label_file = args[2]
    if args[3] == '-l':
        label_file = args[4]
    if args[1] == '-d':
        data_file = args[2]
    if args[3] == '-d':
        data_file = args[4]

    cross_count = 10
    k_num = 5
    rate = 0
    rate_all = 0

    dmr_set, dmr_attr_avg = load_dmr(label_file, data_file)

    if args[5] == '-c':
        dist_func = get_dist_cosine
        dmr_attr_avg = None
    elif args[5] == '-a':
        dist_func = get_dist_adjust_cosine
    elif args[5] == '-e':
        dist_func = get_dist_euclidean
        dmr_attr_avg = None
    else:
        dist_func = get_dist_pearson

    print("Start cross_validation Test:")

    rate_max, k_max = 0, 0

    for k_num in range(1, int(math.sqrt(len(dmr_set)))):
        temp_rate = test_cross_count(
            cross_count, dist_func, k_num, dmr_set, dmr_attr_avg)
        if temp_rate > rate_max:
            rate_max = temp_rate
            k_max = k_num
        print(
            'ten fold cross validation(knn: k =',
            str(k_num),
            '): %.3f' %
            temp_rate)

    print('ten fold cross validation test finished.')
    print('best k for the test data:', k_max)

    print('Start loocv test:')
    count_max, k_max = 0, 0
    for k_num in range(1, int(math.sqrt(len(dmr_set)))):
        count = 0
        for data in dmr_set:
            count += 1 if loocv(data, dist_func, dmr_set, k_num, dmr_attr_avg) else 0
        if count > count_max:
            count_max = count
            k_max = k_num
        print('k =', str(k_num))
        print('num of data:', len(dmr_set))
        print('passed:', count, '%.2f%%' % (count * 100 / len(dmr_set)))

    print('loocv test finished.')
    print('best k for the loocv test:', k_max)
