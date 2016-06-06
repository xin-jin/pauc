#pragma once

#include <limits>
#include <cassert>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <stack>

#include "include/thread-pool.hpp"

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

        SearchResult(): m(std::numeric_limits<PriceT>::lowest()),
                        m2(m), best_item(-1), second_item(-1),
                        payoff(0), second_payoff(0) {}
    };

    Assignment(std::string filename, size_t nsim, size_t nblock);
    ~Assignment() {}

    /** Run the auction algorithm */
    void auction();
    /** Print the final results */
    void printAssignment(bool summary_only);
private:
    Assignment() = delete;

    void searchBid(IdxT i, size_t start, size_t end, SearchResult& sr, std::mutex& mt_sr);
    void bid(IdxT i, size_t sim_id);

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

    size_t niter_;              // records the number of itereations
};
