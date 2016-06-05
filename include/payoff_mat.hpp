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

    PayoffMat(std::string filename);
    void printMat();
	
private:
    PayoffMat() = delete;

    template <typename T>
    inline void tabPrint(T dat, int width = 8);

    IdxT n_;
    MatT mat_;
};
