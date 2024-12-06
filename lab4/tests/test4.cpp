#include <gtest/gtest.h>
#include <dlfcn.h>
#include <cmath>
#include <cstdlib>
#include <cstring>

extern "C" {
    float E(int x);
    char* translation(long x);
}

// Вспомогательное для TestEFunctionLib2
long long factorial(int n) {
    if (n == 0 || n == 1) return 1;
    return n * factorial(n - 1);
}

// E - Реализация 1
TEST(TestLibrary1, TestEFunctionLib1) {
    void* handle = dlopen("./liblib1.so", RTLD_LAZY);
    ASSERT_NE(handle, nullptr) << "Ошибка загрузки библиотеки: " << dlerror();

    auto E_func = (float (*)(int))dlsym(handle, "E");
    ASSERT_NE(E_func, nullptr) << "Не удалось найти функцию E";

    int x = 10;
    float result = E_func(x);
    float expected = std::pow(1.0 + 1.0 / x, x);
    EXPECT_NEAR(result, expected, 0.0001);

    dlclose(handle);
}

// Перевод в двоичную
TEST(TestLibrary1, TestTranslationBinaryLib1) {
    void* handle = dlopen("./liblib1.so", RTLD_LAZY);
    ASSERT_NE(handle, nullptr) << "Ошибка загрузки библиотеки: " << dlerror();

    auto translation_func = (char* (*)(long))dlsym(handle, "translation");
    ASSERT_NE(translation_func, nullptr) << "Не удалось найти функцию translation";

    long x = 5;
    char* result = translation_func(x);
    EXPECT_STREQ(result, "101");
    free(result);

    dlclose(handle);
}

// Е - Реализация 2
TEST(TestLibrary2, TestEFunctionLib2) {
    void* handle = dlopen("./liblib2.so", RTLD_LAZY);
    ASSERT_NE(handle, nullptr) << "Ошибка загрузки библиотеки: " << dlerror();

    auto E_func = (float (*)(int))dlsym(handle, "E");
    ASSERT_NE(E_func, nullptr) << "Не удалось найти функцию E";

    int x = 5;
    float result = E_func(x);
    float expected = 1.0;
    for (int i = 1; i <= x; ++i) {
        expected += 1.0 / factorial(i);
    }
    EXPECT_NEAR(result, expected, 0.0001);

    dlclose(handle);
}

// Перевод в троичную
TEST(TestLibrary2, TestTranslationTernaryLib2) {
    void* handle = dlopen("./liblib2.so", RTLD_LAZY);
    ASSERT_NE(handle, nullptr) << "Ошибка загрузки библиотеки: " << dlerror();

    auto translation_func = (char* (*)(long))dlsym(handle, "translation");
    ASSERT_NE(translation_func, nullptr) << "Не удалось найти функцию translation";

    long x = 5;
    char* result = translation_func(x);
    EXPECT_STREQ(result, "12"); // 5 в троичной системе
    free(result);

    dlclose(handle);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}