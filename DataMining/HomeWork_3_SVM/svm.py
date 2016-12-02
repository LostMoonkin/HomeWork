"""
Created by Yemeibeining

11.30.2016
"""

import cv2
import numpy as np
import sys
import re
import math
import matplotlib.pyplot as plt


class Dlbcl:

    def __init__(self, label, data, id):
        self.label = label
        self.data = data
        self.id = id


def wrong_input():
    print('Wrong Input.')
    print(
        'Use python3 svm.py [-l] {label file} [-d] {data file} to run the code.\n')
    sys.exit(-1)


def load_data(label_file, data_file):

    data_set = []

    try:
        with open('./data/label.txt', 'r') as f:
            s = f.read()
            labels = re.split(' ', s)
            count = len(labels)
            for i in range(count):
                data_set.append(Dlbcl(labels[i], [], i))
    except:
        wrong_input()
        return

    try:
        with open('./data/data.txt', 'r') as f:
            data = f.readline().replace('\n', '')

            while data:
                data = re.split('\t', data)
                for i in range(count):
                    data_set[i].data.append(math.log(int(data[i])))
                data = f.readline().replace('\n', '')
    except:
        wrong_input()

    return data_set


def svm_test(data, label, datas, labels):

    train_labels = np.vstack(np.int32(np.array(labels)))

    train_data = np.array(np.float32(datas))

    svm = cv2.ml.SVM_create()
    svm.setType(cv2.ml.SVM_C_SVC)
    svm.setKernel(cv2.ml.SVM_LINEAR)
    svm.setTermCriteria((cv2.TERM_CRITERIA_MAX_ITER, 100, 0))

    svm.train(train_data, cv2.ml.ROW_SAMPLE, train_labels)

    data = np.array(np.float32([data]))

    pre, res = svm.predict(data)

    # print('predict: ', int(res[0][0]), ' origin: ', label)

    return str(int(res[0][0])) == label


# def get_pac_mat(datas, percentage=0.99):
#
#     def zero_mean(data_mat):
#         mean_val = np.mean(data_mat, axis=0)
#         new_data = data_mat - mean_val
#         return new_data, mean_val
#
#     def percentage2n(eig_vals, percentage):
#         sort_array = np.sort(eig_vals)
#         sort_array = sort_array[-1::-1]
#         array_sum = sum(sort_array)
#         tmp_sum = 0
#         num = 0
#         for i in sort_array:
#             tmp_sum += i
#             num += 1
#             if tmp_sum >= array_sum * percentage:
#                 return num
#
#     data_mat, mean_val = zero_mean(np.mat(datas))
#     cov_mat = np.cov(data_mat, rowvar=0)
#     eig_vals, eig_vects = np.linalg.eig(np.mat(cov_mat))
#     n = percentage2n(eig_vals, percentage)
#     eig_val_indice = np.argsort(eig_vals)
#     n_eig_val_indice = eig_val_indice[-1:-(n + 1):-1]
#     n_eigVect = eig_vects[:, n_eig_val_indice]
#     lowDDataMat = data_mat * n_eigVect
#     reconMat = (lowDDataMat * n_eigVect.T) + mean_val
#     return lowDDataMat, reconMat


def get_pca_data(data_set, n=0):

    datas = []
    labels = []

    for i in range(len(data_set)):
        datas.append(data_set[i].data)
        labels.append(data_set[i].label)

    new_data = np.array(np.float32(datas))

    mean, eigenvectors = cv2.PCACompute(
        new_data, np.mean(
            new_data, axis=0).reshape(
            1, -1), cv2.PCA_DATA_AS_ROW, maxComponents=n)
    train_data = cv2.PCAProject(new_data, mean, eigenvectors)

    return labels, train_data


def loocv(data_set, n=0):

    count = len(data_set)
    passed = 0

    labels, train_data = get_pca_data(data_set, n)

    for i in range(len(data_set)):
        test_data = train_data[i]
        new_train_data = np.delete(train_data, i, 0)
        new_labels = labels[:]
        label = labels[i]
        new_labels.pop(i)
        passed += 1 if svm_test(test_data, label,
                                new_train_data, new_labels) else 0

    return passed / count


def draw_res_bar(res):

    n_group = len(res)
    used = set()

    rates = list(map(lambda x: x * 100, res))
    plt.figure(figsize=(n_group, 10))

    x = [str(i) for i in range(n_group)]
    x[0] = 'Default'
    plt.xticks([i for i in range(n_group)], x)
    plt.bar([i for i in range(n_group)], rates, width=0.5,
            facecolor='lightskyblue', align='center')

    for xx, yy in zip([i for i in range(n_group)], rates):
        s = '%.2f' % yy
        plt.text(xx + 0.1, yy + 1, '%.2f' % yy, ha='center', va='bottom')
        used.add(s)

    plt.xlabel('N')
    plt.ylabel('%')
    plt.title('不同筛选维度的分类准确率')

    plt.savefig('./bar.png')
    plt.show()


def draw_res_line(res):

    plt.plot([i for i in range(len(res))], res)
    plt.show()


if __name__ == '__main__':

    args = sys.argv
    label_file, data_file = '', ''

    if len(args) < 4:
        wrong_input()

    if args[1] == '-l':
        label_file = args[2]
    if args[3] == '-l':
        label_file = args[4]
    if args[1] == '-d':
        data_file = args[2]
    if args[3] == '-d':
        data_file = args[4]

    data_set = load_data(label_file, data_file)
    res = []

    for n in range(len(data_set)):
        res.append(loocv(data_set, n))
        print(str(n), 'completed.', str(res[n]))

    draw_res_bar(res)
    # draw_res_line(res)
