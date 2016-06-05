#pragma once

#include <limits>
#include <cassert>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <stack>

#include "thread-pool.hpp"

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

    struct SearchResult {
        PriceT m;
        PriceT m2;
        IdxT best_item;

        SearchResult() {
            m = std::numeric_limits<PriceT>::min();
            m2 = m;
            best_item = -1;
        }
    };

    Assignment(std::string filename, size_t nthreads = 1): thpool(nthreads) {
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

    ~Assignment() {}

    void searchBid(IdxT i, size_t start, size_t end, SearchResult& sr) {
        for (size_t j = start; j != end; ++j) {
            const EdgeT& edge = mat_[i][j];
            PriceT net_payoff = edge.second - p_[edge.first];
            if (net_payoff > sr.m) {
                sr.m = net_payoff;
                sr.best_item = edge.first;
            }
            else {
                sr.m2 = std::max(net_payoff, sr.m2);
            }
        }
        assert(sr.best_item >= 0);
    }

    SearchResult MergeSearchResults(const std::vector<SearchResult>& srv) {
        SearchResult sr;

        for (const auto& s : srv) {
            if (s.m > sr.m) {
                sr.m = s.m;
                sr.best_item = s.best_item;
            }
            else if (s.m > sr.m2) {
                sr.m2 = s.m;
            }
            if (s.m2 > sr.m2) {
                sr.m2 = s.m2;
            }
        }
        return sr;
    }

    void bid(IdxT i) {
        std::vector<SearchResult> srv(thpool.size());
        size_t p_size = n_ / thpool.size(); // partition size
        for (int k = 0; k != thpool.size(); ++k) {
            thpool.schedule([this, i, start = p_size * k,
                             end = std::min(mat_[i].size(), p_size * (k+1)), &sr = srv[k]]{
                                searchBid(i, start, end, sr);
                            });
        }
        thpool.wait();

        SearchResult sr = MergeSearchResults(srv);

        // update bid and reassign item
        IdxT previous_owner = belong_[sr.best_item];
        if (previous_owner != -1) {
            // if someone already owns the item, kick him/her out
            assign_[previous_owner] = -1;
            unassigned_.push(previous_owner);
        }
        assign_[i] = sr.best_item;
        belong_[sr.best_item] = i;
        p_[sr.best_item] += sr.m - sr.m2 + ep_;
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

    ThreadPool thpool;

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
