#include <iostream>
#include <math.h>

using namespace std;

void
get_args(int n, double ** mat, double * diagonal, double * up, double * down) {

    for (int i = 0; i < n; i++) {
        diagonal[i] = mat[i][i];
    }
    for (int i = 0; i < n - 1; i++) {
        up[i] = mat[i][i + 1];
    }
    for (int i = 1; i < n; i++) {
        down[i] = mat[i][i - 1];
    }
}

double *
thomas(int n, double * diagonal, double * up, double * down, double * b) {

    double * p, * q, *res;

    p = (double *) calloc(n, sizeof(double));
    q = (double *) calloc(n, sizeof(double));
    res = (double *) calloc(n, sizeof(double));

    p[0] = diagonal[0];
    for (int i = 0; i < n - 1; i++) {
        q[i] = up[i] / p[i];
        p[i + 1] = diagonal[i + 1] - down[i + 1] * q[i];
    }

    res[0] = b[0] / p[0];
    for (int i = 1; i < n; i++) {
        res[i] = (b[i] - down[i] * res[i - 1]) / p[i];
    }
    for (int i = n - 2; i >= 0; i --) {
        res[i] = res[i] - q[i] * res[i + 1];
    }

    return res;
}



int
main() {

    int n;
    double ** mat, * diagonal, * up, * down, * b;

    cout << "Input N:";
    cin >> n;

    mat = (double **) calloc(n, sizeof(double *));
    b = (double *) calloc(n, sizeof(double));
    diagonal = (double *) calloc(n, sizeof(double));
    up = (double *) calloc(n, sizeof(double));
    down = (double *) calloc(n, sizeof(double));

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

    get_args(n, mat, diagonal, up, down);

    double * res = thomas(n, diagonal, up, down, b);

    cout << "Ans:" << endl;
    for (int i = 0; i < n; i++) {
        cout << res[i] << endl;
    }
    return 0;
}
