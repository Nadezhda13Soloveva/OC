#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include <thread>
#include <chrono>

// Функция для вызова программы parent
int run_parent(const std::string& input, const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        return -1;
    }
    file << input;
    file.close();

    std::string command = "../../build/lab1/parent < " + filename;
    int result = system(command.c_str());

    std::this_thread::sleep_for(std::chrono::seconds(2));

    return result;
}

// Тест успешной записи в файл
TEST(ParentProgramTest, SuccessfulWriteToFile) {
    std::string input = "file.txt\n100 2 5\n13 2\n\n";
    int exit_code = run_parent(input, "test_input.txt");

    std::ifstream result_file("file.txt");
    ASSERT_TRUE(result_file.is_open());

    std::string result1, result2;
    std::getline(result_file, result1);
    std::getline(result_file, result2);
    result_file.close();

    EXPECT_EQ(result1, "10.00");
    EXPECT_EQ(result2, "6.50");

    std::remove("file.txt");
    std::remove("test_input.txt");

    EXPECT_EQ(exit_code, 0);
}

// Тест на деление на 0
TEST(ParentProgramTest, DivisionByZeroError) {
    std::string input = "file.txt\n90 9 20\n13 0\n5\n";
    int exit_code = run_parent(input, "test_input.txt");

    std::ifstream result_file("file.txt");
    ASSERT_TRUE(result_file.is_open());

    std::string result;
    std::getline(result_file, result);
    result_file.close();

    EXPECT_EQ(result, "0.50");

    std::remove("file.txt");
    std::remove("test_input.txt");

    EXPECT_NE(exit_code, 0);  
}

// Тест на некорректные символы
TEST(ParentProgramTest, InvalidCharacterError) {
    std::string input = "file.txt\n24 4 4\n16 g\n8\n";
    int exit_code = run_parent(input, "test_input.txt");

    std::ifstream result_file("file.txt");
    ASSERT_TRUE(result_file.is_open());

    std::string result;
    std::getline(result_file, result);
    result_file.close();

    EXPECT_EQ(result, "1.50");

    std::remove("file.txt");
    std::remove("test_input.txt");

    EXPECT_NE(exit_code, 0);  
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
