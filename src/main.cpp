#include <iostream>
#include "matrix.h"

int main() {
    matrix<size_t, 0> matrix;

    for (auto i = 0; i < 10; ++i) {
        matrix[i][i]     = i;
        matrix[i][9 - i] = i;
    }

    for (auto i = 1; i < 9; ++i) {
        for (auto j = 1; j < 9; ++j) {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << "matrix size = " << matrix.size() << std::endl;

    std::cout << std::endl;
    for (auto v : matrix) {
        size_t x, y, val;
        std::tie(x, y, val) = v;
        std::cout << "[" << x << "][" << y << "] = " << val << std::endl;
    }
    return 0;
}
