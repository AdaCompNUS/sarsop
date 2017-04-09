// written by png shao wei

/*
 *sla.h contains a data structure called SparseMatrix.
 *It is used to store the sparse version of matrices.
 *You can think of it as consisting of a vector that contains three items,
 *i.e the start-state, the end-state and the probability
 *In my factored parser, i need to store similar entries, but i need to sort all the entries.
 *Instead of writing a sort method, I created a class PreSparseMatrix and I implemented a comparator.
 *This allows me to use the default sort provided by STL library.
 *
 *
 **/

#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>
#include "MathLib.h"
#include "PreCEntry.h"

using namespace momdp;
using namespace std;

class PreSparseMatrix {
    public:

        int value_a;
        int value_b;

        PreSparseMatrix();
        PreSparseMatrix(int a, int b);

        vector<PreCEntry> preCEntries;
        void addEntries(int a, int b, double c);
        void sortEntries();

        SharedPointer<SparseMatrix>  convertSparseMatrix();

        std::ostream& write(std::ostream& out);
        void removeDuplicates();

};
