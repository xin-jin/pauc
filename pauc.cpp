#include <getopt.h>
#include "assignment.hpp"
#include "payoff_mat.hpp"

struct Options {
    bool print_only = false;
    std::string filename = "graph.in";
};

void parseArgumentList(int argc, char *argv[], Options& options) {
    struct option ops[] = {
        {"print", no_argument, NULL, 'p'},
        {"file", required_argument, NULL, 'f'},
        {NULL, 0, NULL, 0},
    };

    while (true) {
        int ch = getopt_long(argc, argv, "pf:", ops, NULL);
        if (ch == -1) break;
        switch (ch) {
        case 'p':
            options.print_only = true;
            break;
        case 'f':
            options.filename = optarg;
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
    Assignment assignment(options.filename);
    assignment.auction();
    assignment.printAssignment();

    return 0;
}
