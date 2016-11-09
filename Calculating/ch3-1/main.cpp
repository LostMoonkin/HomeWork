#include <iostream>
#include <math.h>

using namespace std;

#define IS_DEBUG 1

const double eps = 1e-6;
const int inf = 0x3f3f3f3f;

void PrintMatrix(double ** matrix, int n, int m, int pivot) {
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < m; i++) {
            if (i == pivot && j == pivot) {
                cout << '[' << matrix[j][i] << "] ";
            } else {
                cout << matrix[j][i] << " ";
            }
        }
        cout << endl;
    }
    cout << endl;
}

int Gauss(double ** matrix, int n, int m) {
    for (int k = 0; k < n - 1; k++) {
        if (fabs(matrix[k][k]) < eps) {
            cout << "Error: pivot is zero!" << endl;
            return -1;
        }
        for (int i = k + 1; i < n; i++) {
            double coef = matrix[i][k] / matrix[k][k];

            for (int j = k + 1; j < m; j++) {
                matrix[i][j] -= coef * matrix[k][j];
            }

        }
        if (IS_DEBUG) {
            PrintMatrix(matrix, n, m, k);
        }
    }

    return 0;
}

int GaussPivot(double ** matrix, int n, int m) {

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
        if (IS_DEBUG) {
            PrintMatrix(matrix, n, m, k);
        }
    }

    return 0;
}

void BackSub(double ** matrix, double * result, int n, int m) {
    for (int i = n - 1; i >= 0; i--) {
        result[i] = matrix[i][m - 1];

        for (int j = i + 1; j < n; j++) {
            result[i] -= matrix[i][j] * result[j];
        }

        result[i] /= matrix[i][i];
    }
}

int main() {
    double ** mat, ** mat_pivot;
    double * res;
    int n, m;

    cout << "Input n:";
    cin >> n;
    m = n + 1;

    mat = (double **) calloc(n, sizeof(double *));
    mat_pivot = (double **) calloc(n, sizeof(double *));

    for (int i = 0 ; i < n; i++) {
        mat[i] = (double *) calloc(m, sizeof(double));
        mat_pivot[i] = (double *) calloc(m, sizeof(double));

        for (int j = 0; j < m; j++) {
            scanf("%lf", &mat[i][j]);
            mat_pivot[i][j] = mat[i][j];
        }
    }

    res = (double *) calloc(n, sizeof(double));

    cout << "Gauss Elimination:" << endl;
    if (!Gauss(mat, n, m)) {

        BackSub(mat, res, n, m);
        cout << "result:" << endl;
        for (int i = 0; i < n; i++) {
            cout << res[i] << endl;
        }
    }

    memset(res, 0, n);
    cout << "\nPivot Gauss Elimination:" << endl;
    if (!GaussPivot(mat_pivot, n, m)) {

        BackSub(mat_pivot, res, n, m);
        cout << "result:" << endl;
        for (int i = 0; i < n; i++) {
            cout << res[i] << endl;
        }
    }

    return 0;
}