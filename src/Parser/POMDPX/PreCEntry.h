// written by png shao wei

#ifndef PRECENTRY_H
#define PRECENTRY_H

class PreCEntry {
    public:
        int first;
        int second;
        double third;

        PreCEntry(int f, int s, double t);

        void putCEntry(int f, int s, double t);
        bool operator<(const PreCEntry &rhs) const;
        bool operator==(const PreCEntry &rhs) const;
};

#endif
