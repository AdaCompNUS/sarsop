// written by png shao wei

#include "SparseEntry.h"

bool SparseEntry::hasOnlyZeroUI() {

    for (unsigned int i = 0; i < uniqueIndex.size(); i++) {
        if (fabs(uniqueIndex[i].value) > 0.000001)
            return false;
    }
    return true;
}

bool SparseEntry::operator==(const SparseEntry &rhs) const {

    vector<UniqueIndex> a_ui = this->uniqueIndex;
    vector<UniqueIndex> b_ui = rhs.uniqueIndex;

    for (unsigned int i = 0; i < a_ui.size(); i++) {
        if (a_ui[i].index != b_ui[i].index)
            return false;
    }
    return true;
}

bool SparseEntry::operator<(const SparseEntry &rhs) const {

    vector<UniqueIndex> a_ui = this->uniqueIndex;
    vector<UniqueIndex> b_ui = rhs.uniqueIndex;

    for (unsigned int i = 0; i < a_ui.size(); i++) {
        if (a_ui[i].index < b_ui[i].index)
            return true;
        if (a_ui[i].index > b_ui[i].index)
            return false;
    }
    return false;
}

std::ostream& SparseEntry::write(std::ostream& out) {
    out << "Unique Index: " << endl;
    for (unsigned int i = 0; i < uniqueIndex.size(); i++) {
        out << uniqueIndex[i].index << "=" << uniqueIndex[i].value << " ";
    }
    out << endl;
    return out;
}

