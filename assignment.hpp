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
        IntT payoff;

        SearchResult() {
            m = std::numeric_limits<PriceT>::lowest();
            m2 = m;
            best_item = -1;
        }
    };

    Assignment(std::string filename, size_t nthreads): thpool(nthreads) {
        cout << thpool.size() << endl;
        std::ifstream infile(filename);

        IdxT i, j, w;

        infile >> n_;
        belong_.resize(n_, -1);
        assign_.resize(n_, -1);
        payoff_.resize(n_, -1);
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
                sr.m2 = sr.m;
                sr.m = net_payoff;
                sr.best_item = edge.first;
                sr.payoff = edge.second;
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
                sr.m2 = sr.m;
                sr.m = s.m;
                sr.best_item = s.best_item;
                sr.payoff = s.payoff;

                if (s.m2 > sr.m2) {
                    sr.m2 = s.m2;
                }
            }
            else if (s.m > sr.m2) {
                sr.m2 = s.m;
            }
        }

        return sr;
    }

    void bid(IdxT i) {
        size_t np = std::min(thpool.size(), n_);     // # of partitions
        size_t p_size = (mat_[i].size()-1) / np + 1; // partition size
        std::vector<SearchResult> srv(np);
        size_t start = 0;
        size_t end = p_size;

        for (int k = 0; k != np; ++k) {
            thpool.schedule([this, i, start,
                             end, &sr = srv[k]]{
                                searchBid(i, start, end, sr);
                            });
            start = end;
            end += p_size;
            end = std::min(end, mat_[i].size());
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
        payoff_[i] = sr.payoff;
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
        double tp = 0;
        for (IdxT i = 0; i != n_; ++i) {
            tp += payoff_[i];
            cout << i << " gets " << assign_[i] << endl;
        }
        cout << "Total payoff is " << tp << endl;
    }

private:
    Assignment() = delete;

    ThreadPool thpool;

    // assign_[i] indicates the item assigned to person i
    // belong_[j] indicates the person to which item j belongs
    std::vector<IdxT> assign_, belong_;
    // the persons who are unassigned
    std::stack<IdxT> unassigned_;
    std::vector<IntT> payoff_;
    PriceT ep_;
    IdxT n_;
    PVecT p_;
    MatT mat_;
};
