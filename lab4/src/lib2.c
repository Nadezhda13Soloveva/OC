#include <stdlib.h>
#include <string.h>

// Факториал для реализации 2 функции E
static long long factorial(int n) {
    if (n == 0 || n == 1) return 1;
    return n * factorial(n - 1);
}

// Реализация 2 функции E
float E(int x) {
    float sum = 0.0f;
    for (int i = 0; i <= x; ++i) {
        sum += 1.0 / factorial(i);
    }
    return sum;
}

// Реализация 2 функции перевода в другую систему счисления (троичная)
char* translation(long x) {
    char* result = (char*)malloc(65); 
    if (!result) return NULL;
    result[64] = '\0';
    int index = 63;
    do {
        result[index--] = '0' + (x % 3);
        x /= 3;
    } while (x > 0);
    return strdup(&result[index + 1]);
}
