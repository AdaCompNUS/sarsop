// written by png shao wei
#ifndef SPARSEENTRY_H
#define SPARSEENTRY_H

#include <cmath>
#include <vector>
#include "UniqueIndex.h"
#include <iostream>

using namespace std;

class SparseEntry {

    public:
        vector<UniqueIndex> uniqueIndex;

        bool hasOnlyZeroUI();

        //SparseEntry operator=(const SparseEntry &rhs);
        //Need to overload these operators to use STL stable_sort
        bool operator==(const SparseEntry &rhs) const;
        bool operator<(const SparseEntry &rhs) const;

        std::ostream& write(std::ostream& out);
};

#endif
