#include "assignment.hpp"
#include "payoff_mat.hpp"


int main() {
    std::string filename = "graph.in";

    Assignment assignment(filename);

    // pmat.printMat();
    assignment.auction();
    assignment.printAssignment();

    return 0;
}
