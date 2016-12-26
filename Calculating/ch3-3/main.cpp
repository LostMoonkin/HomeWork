#include <iostream>
#include <math.h>

using namespace std;

const double esp = 1e-6;

int
compare(int n, double * x, double * pre_x) {

    double res = 0;
    for (int i = 0; i < n; i++) {
        res += fabs(x[i] - pre_x[i]);
    }

    return res <= esp;
}

int
jacobi(int n, double ** mat, double * x, double * b) {

    static double * pre_x;
    static int count = 0;

    count ++;
    pre_x = (double *) calloc(n, sizeof(double));

    for (int i = 0; i < n; i++) {
        pre_x[i] = x[i];
    }

    for (int i = 0; i < n; i++) {
        double sum = 0;
        for (int j = 0; j < n; j++) {
            if (j != i) {
                sum += mat[i][j] * pre_x[j];//用前一次的pre_x迭代计算
            }
        }
        x[i] = (b[i] - sum) / mat[i][i];
    }

    if (!compare(n, x, pre_x)) {
        jacobi(n, mat, x, b);
    }
    return count;
}

int gauss(int n, double ** mat, double * x, double * b) {

    static double * pre_x;
    static int count = 0;

    count ++;
    pre_x = (double *) calloc(n, sizeof(double));

    for (int i = 0; i < n; i++) {
        pre_x[i] = x[i];
    }

    for (int i = 0; i < n; i++) {
        double sum = 0;
        for (int j = 0; j < n; j++) {
            if (j != i) {
                sum += mat[i][j] * x[j];//用本次的x迭代计算
            }
        }
        x[i] = (b[i] - sum) / mat[i][i];
    }

    if (!compare(n, x, pre_x)) {
        gauss(n, mat, x, b);
    }
    return count;

}


int
main() {

    int n;
    double ** mat, * x, * b;

    cout << "Input N:";
    cin >> n;

    mat = (double **) calloc(n, sizeof(double *));
    x = (double *) calloc(n, sizeof(double));
    b = (double *) calloc(n, sizeof(double));

    cout << "Input N X N Matrix a:" << endl;
    for (int i = 0; i < n; i++) {
        mat[i] = (double *)calloc(n, sizeof(double));

        for (int j = 0; j < n; j++) {
            cin >> mat[i][j];
        }
    }

    cout << "Input 1 X N Matrix b:" << endl;
    for (int i = 0; i < n; i++) {
        cin >> b[i];
    }

    cout << endl << "Exp = " << esp << ", jacobi:" << endl;
    int count = jacobi(n, mat, x, b);

    cout << "Ans:" << endl;
    for (int i = 0; i < n; i++) {
        cout << x[i] << endl;
    }
    cout << "Count: " << count << endl;

    memset(x, 0, sizeof(double) * n);

    cout << "Exp = " << esp << ", gauss:" << endl;
    count = gauss(n, mat, x, b);

    cout << "Ans:" << endl;
    for (int i = 0; i < n; i++) {
        cout << x[i] << endl;
    }
    cout << "Count: " << count << endl;
    return 0;
}