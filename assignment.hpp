#pragma once

#include <limits>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <stack>

using PriceT = double;
using IntT = int;
using PVecT = std::vector<PriceT>;
using IdxT = int;
using std::cout;
using std::endl;
using MatT = std::vector<std::vector<IntT>>;

class Assignment {
public:
    Assignment(std::string filename) {
        std::ifstream infile(filename);

        IdxT i, j, w;

        infile >> n_;
        belong_.resize(n_, -1);
        assign_.resize(n_, -1);
        mat_.resize(n_);
        p_.resize(n_, 0);
        for (auto& v : mat_) {
            v.resize(n_, -1);
        }

        while (infile >> i >> j >> w) {
            mat_[i][j] = w;
        }

        for (IdxT j = n_-1; j >= 0; --j) {
            unassigned_.push(j);
        }

        ep_ = 1.0/(n_+1);
    }


    void bid(IdxT i) {
        PriceT m = mat_[i][0] - p_[0], m2 = m;
        IdxT best_item = 0;

        for (IdxT j = 1; j != n_; ++j) {
            if (mat_[i][j] - p_[j] > m) {
                m = mat_[i][j];
                best_item = j;
            }
            else {
                m2 = std::max(mat_[i][j] - p_[j], m2);
            }
        }

        // update bid and reassign item
        IdxT previous_owner = belong_[best_item];
        if (previous_owner != -1) {
            // if someone already owns the item, kick him/her out
            assign_[previous_owner] = -1;
            unassigned_.push(previous_owner);
        }
        assign_[i] = best_item;
        belong_[best_item] = i;
        p_[best_item] += m - m2 + ep_;
    }

    void auction() {
        while (!unassigned_.empty()) {
            IdxT bidder = unassigned_.top();
            unassigned_.pop();
            bid(bidder);
        }
    }

    void printAssignment() {
        for (IdxT i = 0; i != n_; ++i) {
            cout << i << " gets " << assign_[i] << endl;
        }
    }

private:
    Assignment() = delete;

    // assign_[i] indicates the item assigned to person i
    // belong_[j] indicates the person to which item j belongs
    std::vector<IdxT> assign_, belong_;
    // the persons who are unassigned
    std::stack<IdxT> unassigned_;
    PriceT ep_;
    IdxT n_;
    PVecT p_;
    MatT mat_;
};
