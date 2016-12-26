#include <iostream>

using namespace std;

double
lagrange(int n, double * x, double * y, double cal_x) {

    double res = 0, tmp;

    for (int i = 0; i < n; i++) {
        tmp = y[i];

        for (int j = 0; j < n; j++) {
            if (j != i) {
                tmp *= (cal_x - x[j]) / (x[i] - x[j]);
            }
        }
        res += tmp;
    }

    return res;
}

double
newtown(int n, double * x, double * y, double cal_x) {

    double * tmp = (double *) calloc(n, sizeof(double));
    double * yy = (double *) calloc(n, sizeof(double));
    double res = y[0], xx = 1;

    memcpy(yy, y, sizeof(double) * n);

    for (int i = 1; i < n; i++) {
        for (int j = i; j < n; j++) {
            tmp[j] = (yy[j] - yy[j - 1]) / (x[j] - x[j - i]); // 差商
        }
        xx *= (cal_x - x[i - 1]);
        res += xx * tmp[i];

        for (int j = i; j < n; j++) {
            yy[j] = tmp[j];
        }
    }

    return res;

}

int
main() {

    int n = 6;
    double x[] = {0.3, 0.42, 0.5, 0.58, 0.66, 0.72}, y[] = {1.04403, 1.08462, 1.11803, 1.15603, 1.19817, 1.23223};
    double cal_x[] = {0.46, 0.55, 0.60};

    cout << "lagrange:" << endl;
    for (int i = 0; i < 3; i++) {
        printf("x: %.2lf, y: %.20f\n", cal_x[i], lagrange(n, x, y, cal_x[i]));
    }

    cout << endl << "newtown:" << endl;
    for (int i = 0; i < 3; i++) {
        printf("x: %.2lf, y: %.20f\n", cal_x[i], newtown(n, x, y, cal_x[i]));
    }

    return 0;
}