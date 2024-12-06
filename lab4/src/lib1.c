#include <math.h>
#include <stdlib.h>
#include <string.h>

// Реализация 1 функции E
float E(int x) {
    return pow(1.0 + 1.0 / x, x);
}

// Реализация 1 функции перевода в другую систему счисления (двоичная)
char* translation(long x) {
    char* result = (char*)malloc(65);
    if (!result) return NULL;
    result[64] = '\0';
    int index = 63;
    do {
        result[index--] = '0' + (x % 2);
        x /= 2;
    } while (x > 0);
    return strdup(&result[index + 1]);
}
