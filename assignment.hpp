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
        IdxT second_item;
        IntT payoff;
        IntT second_payoff;

        SearchResult() {
            m = std::numeric_limits<PriceT>::lowest();
            m2 = m;
            best_item = -1;
            second_item = -1;
        }
    };

    Assignment(std::string filename, size_t nsim, size_t nblock): thpool_(nsim*nblock), nsim_(nsim), nblock_(nblock) {
        std::ifstream infile(filename);

        IdxT i, j, w;

        infile >> n_;
        if (n_ < thpool_.size()) {
            // when the size of data is too small, we just use the sequential version
            nsim_ = 1;
            nblock_ = 1;
        }

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

    void searchBid(IdxT i, size_t start, size_t end, SearchResult& sr, std::mutex& mt_sr) {
        SearchResult tmp_sr;

        for (size_t j = start; j != end; ++j) {
            const EdgeT& edge = mat_[i][j];
            PriceT net_payoff = edge.second - p_[edge.first];
            if (net_payoff > tmp_sr.m) {
                tmp_sr.m2 = tmp_sr.m;
                tmp_sr.second_item = tmp_sr.best_item;
                tmp_sr.second_payoff = tmp_sr.payoff;
                tmp_sr.m = net_payoff;
                tmp_sr.best_item = edge.first;
                tmp_sr.payoff = edge.second;
            }
            else if (net_payoff > tmp_sr.m2) {
                tmp_sr.m2 = net_payoff;
                tmp_sr.second_item = edge.first;
                tmp_sr.second_payoff = edge.second;
            }
        }
        assert(tmp_sr.m >= tmp_sr.m2);
        assert(tmp_sr.best_item >= 0);

        mt_sr.lock();
        if (tmp_sr.m > sr.m) {
            if (sr.m > tmp_sr.m2) {
                sr.m2 = sr.m;
                sr.second_item = sr.best_item;
                sr.second_payoff = sr.payoff;
            }
            else {
                sr.m2 = tmp_sr.m2;
                sr.second_item = tmp_sr.second_item;
                sr.second_payoff = tmp_sr.second_payoff;
            }
            sr.m = tmp_sr.m;
            sr.best_item = tmp_sr.best_item;
            sr.payoff = tmp_sr.payoff;
        }
        else {
            if (tmp_sr.m > sr.m2) {
                sr.m2 = tmp_sr.m;
                sr.second_item = tmp_sr.best_item;
                sr.second_payoff = tmp_sr.payoff;
            }
        }
        assert(sr.m >= sr.m2);
        mt_sr.unlock();
    }

    // [[Deprecated]]
    // SearchResult MergeSearchResults(const std::vector<SearchResult>& srv) {
    //     SearchResult sr;

    //     for (const auto& s : srv) {
    //         if (s.m > sr.m) {
    //             sr.m2 = sr.m;
    //             sr.m = s.m;
    //             sr.best_item = s.best_item;
    //             sr.payoff = s.payoff;

    //             if (s.m2 > sr.m2) {
    //                 sr.m2 = s.m2;
    //             }
    //         }
    //         else if (s.m > sr.m2) {
    //             sr.m2 = s.m;
    //         }
    //     }

    //     return sr;
    // }

    void bid(IdxT i, size_t sim_id) {
        size_t p_size = (mat_[i].size()-1) / nblock_ + 1;  // partition size
        SearchResult sr;
        std::mutex mt_sr;
        size_t start = 0;
        size_t end = p_size;

        size_t worker_end = sim_id + nblock_;

        for (int worker_id = sim_id + 1; worker_id != worker_end; ++worker_id) {
            thpool_.schedule([this, i, start,
                              end, &sr, &mt_sr]{
                                 searchBid(i, start, end, sr, mt_sr);
                             }, worker_id);
            start = end;
            end += p_size;
        }

        // this thread searches the rest
        searchBid(i, start, mat_[i].size(), sr, mt_sr);

        for (int worker_id = sim_id + 1; worker_id != worker_end; ++worker_id) {
            thpool_.wait(worker_id);
        }

        // update price and reassign item
        mt_.lock();
        PriceT new_bid = p_[sr.best_item] + (sr.payoff - p_[sr.best_item]) - (sr.second_payoff - p_[sr.second_item]) + ep_;
        if (new_bid > p_[sr.best_item]) {
            IdxT previous_owner = belong_[sr.best_item];
            if (previous_owner != -1) {
                // if someone already owns the item, kick him/her out
                assign_[previous_owner] = -1;
                unassigned_.push(previous_owner);
            }
            assign_[i] = sr.best_item;
            belong_[sr.best_item] = i;
            payoff_[i] = sr.payoff;
            p_[sr.best_item] = new_bid;
        }
        else {
            unassigned_.push(i);
        }
        mt_.unlock();
    }

    void auction() {
        while (!unassigned_.empty()) {
            for (size_t k = 0; k != nsim_; ++k) {
                mt_.lock();
                if (unassigned_.empty()) break;
                IdxT bidder = unassigned_.top();
                unassigned_.pop();
                mt_.unlock();

                size_t sim_id = k * nblock_;

                thpool_.schedule([this, bidder, sim_id]{
                        bid(bidder, sim_id);
                    }, sim_id);
            }

            for (size_t k = 0; k != nsim_; ++k) {
                thpool_.wait(k * nblock_);
            }
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

    ThreadPool thpool_;
    size_t nsim_;
    size_t nblock_;

    // assign_[i] indicates the item assigned to person i
    // belong_[j] indicates the person to which item j belongs
    std::vector<IdxT> assign_, belong_;
    // the persons who are unassigned
    std::stack<IdxT> unassigned_;
    std::vector<IntT> payoff_;
    // protects price info and assignments info
    std::mutex mt_;
    PriceT ep_;
    IdxT n_;
    PVecT p_;
    MatT mat_;
};
