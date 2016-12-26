'''
create by Yemei Beining
12/24/2016
'''

import matplotlib.pyplot as plt
import math
import numpy

def draw(ax, x, y, mat_a, mat_b, order):
    xxa = numpy.arange(min(x), max(x), 0.01)
    yya = []
    yyb = []
    ax.plot(x, y, color='m', linestyle='', marker='.')

    for i in range(0, len(xxa)):
        yy = 0.0
        for j in range(0, order + 1):
            dy = 1.0
            for k in range(0,j):
                dy *= xxa[i]
            dy *= mat_a[j]
            yy += dy
        yya.append(yy)
    
    for i in range(0, len(xxa)):
        yyb.append(mat_b[0] * math.exp(xxa[i] * mat_b[1]))
    
    ax.plot(xxa, yya, color='g', linestyle='-', marker='') 
    ax.plot(xxa, yyb, color='b', linestyle='-', marker='') 
    ax.legend()  

if __name__ == '__main__':
    x = [
        [1, 1.5, 2, 2.5, 3],
        [3.5, 4, 4.5, 5, 5.5],
        [6, 6.5, 7, 7.5, 8]
    ]
    y = [
        [33.4, 79.5, 122.65, 159.05, 189.15],
        [214.15, 238.65, 252.50, 267.55, 280.50],
        [296.65, 301.40, 310.40, 318.15, 325.15]
    ]
    mat_a = [
        [-78.42, 122.496, -11.0714],
        [-18.3486, 88.6343, -6.25714],
        [252.136, 1.75, 0.928571]
    ]
    mat_b = [
        [18.8499, 0.832286],
        [138.532, 0.13082],
        [222.418, 0.0475104]
    ]

    fig = plt.figure()  
    ax = fig.add_subplot(111)
    for i in range(0, 3):
        draw(ax, x[i], y[i], mat_a[i], mat_b[i], 2)
    plt.show()
