#include <iostream>
#include <math.h>

using namespace std;

const double precision = 1e-10;

double Func_x(double x) {
    return 2.0 - log(x);
}

double Func_fx(double x) {
    return log(x) + x - 2.0;
}

double Func_f_x(double x) {
    return 1.0 / x + 1;
}

double Func_newton(double x) {
    return x - Func_fx(x) / Func_f_x(x);
}

double Iteration(double x, int * count, double (*f)(double), const double eps) {

    double new_x = f(x);
    (*count) ++;

    if (fabs(new_x - x) < eps) {
        return new_x;
    }

    return Iteration(new_x, count, f, eps);

}

double Binary(double left, double right, int *count, double (*f)(double), const double eps) {

    double mid = (left + right) / 2.0;
    double res = f(mid);

    (*count) ++;

    if (fabs(res) < eps) {
        return mid;
    }

    if (res < 0) {
        return Binary(mid, right, count, f, eps);
    }

    if (res > 0) {
        return Binary(left, mid, count, f, eps);
    }

    return mid;
}

double Secant(double x0, double x1, int * count, double (*f)(double), const double eps) {

    double x2 = x1 - f(x1) * (x1 - x0) / (f(x1) - f(x0));

    (*count) ++;

    if (fabs(x2 - x1) < eps) {
        return x2;
    }

    return Secant(x1, x2, count, f, eps);
}


int main() {

    int count = 0;

    double x = Iteration(1.5, &count, &Func_x, precision);

    printf("Simple Iteration:\nres = %.10lf count = %d\n", x, count);

    count = 0;
    x = Binary(1, 2, &count, &Func_fx, precision);

    printf("\nBinary:\nres = %.10lf count = %d\n", x, count);

    count = 0;
    x = Iteration(1.5, &count, &Func_newton, precision);
    printf("\nNewton Iteration:\nres = %.10lf count = %d\n", x, count);

    count = 0;
    x = Secant(1, 2, &count, &Func_fx, precision);
    printf("\nSecant\nres = %.10lf count = %d\n", x, count);

    return 0;
}