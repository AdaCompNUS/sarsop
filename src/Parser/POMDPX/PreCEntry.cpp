// written by png shao wei

#include "PreCEntry.h"

PreCEntry::PreCEntry(int f, int s, double t) {
    first = f;
    second = s;
    third = t;
}

void PreCEntry::putCEntry(int f, int s, double t) {
    first = f;
    second = s;
    third = t;
}

bool PreCEntry::operator<(const PreCEntry &rhs) const {

    // 0 2 1 < 1 3 1
    // 2 8 1 < 3 8 1
    // 2 0 1 < 3 1 1

    if (second < rhs.second)
        return 1;
    else if (second > rhs.second)
        return 0;
    else if (first < rhs.first)
        return 1;
    else
        return 0;
}

bool PreCEntry::operator==(const PreCEntry &rhs) const {

    if ((first == rhs.first) && (second == rhs.second) && (third == rhs.third))
        return 1;
    else
        return 0;
}
