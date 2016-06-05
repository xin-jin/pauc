#pragma once

#include <limits>
#include <cassert>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <stack>

using std::cout;
using std::endl;

class Assignment {
public:
    using PriceT = double;
    using IntT = int;
    using PVecT = std::vector<PriceT>;
    using IdxT = int;
    // first is item index, second is payoff
    using EdgeT = std::pair<IdxT, IntT>;
    using MatT = std::vector<std::vector<EdgeT>>;

    Assignment(std::string filename) {
        std::ifstream infile(filename);

        IdxT i, j, w;

        infile >> n_;
        belong_.resize(n_, -1);
        assign_.resize(n_, -1);
        mat_.resize(n_);
        p_.resize(n_, 0);

        while (infile >> i >> j >> w) {
            mat_[i].push_back({ j, w });
        }

        for (IdxT i = n_-1; i >= 0; --i) {
            unassigned_.push(i);
        }

        ep_ = 1.0/(n_+1);
    }


    void bid(IdxT i) {
        PriceT m = std::numeric_limits<PriceT>::min(), m2 = m;
        IdxT best_item = -1;

        for (const EdgeT& edge : mat_[i]) {
            PriceT net_payoff = edge.second - p_[edge.first];
            if (net_payoff > m) {
                m = net_payoff;
                best_item = edge.first;
            }
            else {
                m2 = std::max(net_payoff, m2);
            }
        }
        assert(best_item >= 0);

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
