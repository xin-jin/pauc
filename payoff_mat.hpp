#pragma once

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>

using std::cout;
using std::endl;


class PayoffMat {
public:
    using IntT = int;
    using IdxT = int;
    using MatT = std::vector<std::vector<IntT>>;

    PayoffMat(std::string filename) {
        std::ifstream infile(filename);

        IdxT i, j, w;

        infile >> n_;
        mat_.resize(n_);
        for (auto& v : mat_) {
            v.resize(n_, -1);
        }

        while (infile >> i >> j >> w) {
            mat_[i][j] = w;
        }

    }

    template <typename T>
    void tabPrint(T dat, int width = 8) {
        cout << std::left << std::setw(width) << std::setfill(' ') << dat;
    }

    void printMat() {
        tabPrint('/');
        for (IdxT i = 0; i != n_; ++i) {
            tabPrint(i);
        }
        cout << endl;
        for (IdxT i = 0; i != n_; ++i) {
            tabPrint(i);
            for (IdxT j = 0; j != n_; ++j) {
                tabPrint(mat_[i][j]);
            }
            cout << endl;
        }
    }

private:
    PayoffMat() = delete;

    IdxT n_;
    MatT mat_;
};
