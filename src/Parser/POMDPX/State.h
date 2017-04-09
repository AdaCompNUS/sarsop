// written by png shao wei
// StateObsAct is for base class
// ObsAct is for Observation and Action
// State inherits StateObsAct and it has additional previous and current values

#ifndef State_H
#define State_H

#include <iostream>
#include <vector>
#include <iterator>

#include "StateObsAct.h"
using namespace std;

class State: public StateObsAct {
    protected:
        string vnamePrev;
        string vnameCurr;
        bool observed;

    public:
        void setVNamePrev(string str);
        void setVNameCurr(string str);
        void setObserved(bool o);
        const string getVNamePrev() const;
        const string getVNameCurr() const;
        const bool getObserved() const;

        std::ostream& write(std::ostream& out);
};

#endif

