#include <iostream>
#include <math.h>

using namespace std;

const double eps = 1e-6;
const int inf = 0x3f3f3f3f;
int order = 2; // 设定曲线阶数为2

/*
 * 使用高斯消元求矩阵解
 */

int
gauss_pivot(double ** matrix, int n, int m) {

    for (int k = 0; k < n; k++) {
        double max_f = -inf + 0.0;
        int i_max = 0;

        for (int i = k; i < n; i++) {
            if (fabs(matrix[i][k]) > max_f) {
                max_f = fabs(matrix[i][k]);
                i_max = i;
            }
        }

        if (fabs(matrix[i_max][k]) < eps) {
            cout << "Error: matrix is singular!" << endl;
            return -1;
        }

        double *tmp = matrix[k];
        matrix[k] = matrix[i_max];
        matrix[i_max] = tmp;

        for (int i = k + 1; i < n; i++) {
            double coef = matrix[i][k] / matrix[k][k];

            for (int j = k; j < m; j++) {
                matrix[i][j] -= coef * matrix[k][j];
            }
        }
    }

    return 0;
}


void
back_sub(double ** matrix, double * result, int n, int m) {

    for (int i = n - 1; i >= 0; i--) {
        result[i] = matrix[i][m - 1];

        for (int j = i + 1; j < n; j++) {
            result[i] -= matrix[i][j] * result[j];
        }

        result[i] /= matrix[i][i];
    }
}

void create_mat(double ** mat, double * x, double * y, int n) {

    for (int i = 0; i <= order; i++) {

        for (int j = 0; j <= order; j++) {

            double tx = 0;
            for (int k = 0; k < n; k++) {

                double dx = 1;
                for (int l = 0; l < j + i; l++) {
                    dx *= x[k];
                }
                tx += dx;
            }
            mat[i][j] = tx;
        }
    }

    for (int i = 0; i <= order; i++) {

        double ty = 0;
        for (int k = 0; k < n; k++) {

            double dy = 1.0;
            for (int l = 0; l < i; l ++) {
                dy *= x[k];
            }
            ty += y[k] * dy;
        }
        mat[i][order + 1] = ty;
    }
}



int
main() {

    double x[3][5] = {
            1, 1.5, 2, 2.5, 3,
            3.5, 4, 4.5, 5, 5.5,
            6, 6.5, 7, 7.5, 8
    };
    double y[3][5] = {
            33.4, 79.5, 122.65, 159.05, 189.15,
            214.15, 238.65, 252.50, 267.55, 280.50,
            296.65, 301.40, 310.40, 318.15, 325.15,
    };

    double ** mat, * res;
    int n = 5;

    mat = (double **) calloc(order + 1, sizeof(double));
    res = (double *) calloc(order + 1, sizeof(double));

    for (int i = 0; i <= order; i++) {
        mat[i] = (double *) calloc(order + 2, sizeof(double));
    }


    cout << "y = a * x^2 + bx + c" << endl;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < order + 1; j++) {
            memset(mat[j], 0, sizeof(double) * (order + 1));
        }

        create_mat(mat, x[i], y[i], n);

//        for (int j = 0; j <= order; j++) {
//            for (int k = 0; k <= order + 1; k++) {
//                cout << mat[j][k] << " ";
//            }
//            cout << endl;
//        }
//        //debug

        gauss_pivot(mat, order + 1, order + 2);
        back_sub(mat, res, order + 1, order + 2);

        cout << "group[" << i + 1 << "]:" << endl;
        for (int j = 0; j < order + 1; j++) {
            cout << res[j] << " ";
        }
        cout << endl;
    }

    cout << "y = a * e ^(k * x)" << endl;
    order = 1;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 5; j++) {
            y[i][j] = log(y[i][j]);
        }
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < order + 1; j++) {
            memset(mat[j], 0, sizeof(double) * (order + 1));
        }

        create_mat(mat, x[i], y[i], n);
        gauss_pivot(mat, order + 1, order + 2);
        back_sub(mat, res, order + 1, order + 2);

        cout << "group[" << i + 1 << "]:" << endl;
        res[0] = exp(res[0]);
        for (int j = 0; j < order + 1; j++) {
            cout << res[j] << " ";
        }
        cout << endl;
    }


    return 0;
}