#include <getopt.h>
#include "assignment.hpp"
#include "payoff_mat.hpp"

struct Options {
    bool print_only = false;
    std::string filename = "graph.in";
    // # of people that simultaneously update bids
    size_t nsim = 1;
    // # of search blocks
    size_t nblock = 1;
    // whether use Gauss-Seidel method
    bool gs = false;
    // if true, then don't print the output; this is mainly for
    // timing purposes
    bool noprint = false;
};

void parseArgumentList(int argc, char *argv[], Options& options) {
    struct option ops[] = {
        {"print-matrix", no_argument, NULL, 'p'},
        {"file", required_argument, NULL, 'f'},
        {"sim", required_argument, NULL, 's'},
        {"block", no_argument, NULL, 'b'},
        {"no-print", no_argument, NULL, 'n'},
        {NULL, 0, NULL, 0},
    };

    while (true) {
        int ch = getopt_long(argc, argv, "pf:s:b:n", ops, NULL);
        if (ch == -1) break;
        switch (ch) {
        case 'p':
            options.print_only = true;
            break;
        case 'f':
            options.filename = optarg;
            break;
        case 's':
            options.nsim = atoi(optarg);
            break;
        case 'b':
            options.nblock = atoi(optarg);
        case 'g':
            options.gs = true;
            break;
        case 'n':
            options.noprint = true;
            break;
        default:
            cout << "Unrecognized or improperly supplied flag." << endl;
            exit(2);
        }
    }

    argc -= optind;
    if (argc > 0) {
        cout << "Too many arguments." << endl;
        exit(2);
    }
}

int main(int argc, char *argv[]) {
    Options options;
    parseArgumentList(argc, argv, options);

    if (options.print_only) {
        PayoffMat pmat(options.filename);
        pmat.printMat();
        return 0;
    }
    Assignment assignment(options.filename, options.nsim, options.nblock);
    assignment.auction();
    if (!options.noprint)
        assignment.printAssignment();

    return 0;
}
