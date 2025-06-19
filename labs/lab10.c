#include <stdio.h>
#include <stdarg.h>

double mean(int count, ...) {
    va_list args;
    va_start(args, count);

    int sum = 0;
    for (int i = 0; i < count; i++) {
        int num = va_arg(args, int);
        sum += num;
    }

    va_end(args);
    return count ? (double)sum / count : 0.0;
}

int main() {
    printf("Mean = %.2f\n", mean(5, 10, 20, 30, 40, 50));
    return 0;
}

