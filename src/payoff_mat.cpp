#include <include/payoff_mat.hpp>


PayoffMat::PayoffMat(std::string filename) {
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
void PayoffMat::tabPrint(T dat, int width) {
    cout << std::left << std::setw(width) << std::setfill(' ') << dat;
}

void PayoffMat::printMat() {
    tabPrint('/');
    for (IdxT i = 0; i != n_; ++i) {
        tabPrint(i);
    }
    cout << endl;
    for (IdxT i = 0; i != n_; ++i) {
        tabPrint(i);
        for (IdxT j = 0; j != n_; ++j) {
            if (mat_[i][j] != -1)
                tabPrint(mat_[i][j]);
            else
                tabPrint('-');
        }
        cout << endl;
    }
}
