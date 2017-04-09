// written by png shao wei

#include "PreCMatrix.h"

PreSparseMatrix::PreSparseMatrix() {
    value_a = 0;
    value_b = 0;
}

PreSparseMatrix::PreSparseMatrix(int a, int b) {
    value_a = a;
    value_b = b;
}

void PreSparseMatrix::addEntries(int a, int b, double c) {
    PreCEntry entry = PreCEntry(a, b, c);
    preCEntries.push_back(entry);
}

void PreSparseMatrix::sortEntries() {
    stable_sort(preCEntries.begin(), preCEntries.end());
}

SharedPointer<SparseMatrix>  PreSparseMatrix::convertSparseMatrix() {
    sortEntries();
    removeDuplicates();
    SharedPointer<SparseMatrix>  temp (new SparseMatrix(value_a, value_b));
    for (unsigned int i = 0; i < preCEntries.size(); i++)
    {
        temp->push_back(preCEntries[i].first, preCEntries[i].second,preCEntries[i].third);
    }
    temp->canonicalize(); // important to call this , if not printout will not work
    return temp;
}

void PreSparseMatrix::removeDuplicates() {

    if (preCEntries.size() > 0) {
        vector<PreCEntry> newCEntries;

        for (unsigned int i = 0; i < preCEntries.size() - 1; i++) {
            if (!(preCEntries[i] == preCEntries[i + 1])) {
                newCEntries.push_back(preCEntries[i]);
            }
        }
        newCEntries.push_back(preCEntries[preCEntries.size() - 1]);

        preCEntries = newCEntries;
    }//end of if

}

std::ostream& PreSparseMatrix::write(std::ostream& out) {
    out << value_a << " " << value_b << endl;
    for (unsigned int i = 0; i < preCEntries.size(); i++)
        out << preCEntries[i].first << " " << preCEntries[i].second << " "
            << preCEntries[i].third << endl;
    return out;
}
