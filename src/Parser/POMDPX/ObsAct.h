// written by png shao wei
// StateObsAct is for base class
// ObsAct is for Observation and Action
// State inherits StateObsAct and it has additional previous and current values

#ifndef ObsAct_H
#define ObsAct_H

#include <iostream>
#include <vector>
#include <iterator>
#include "StateObsAct.h"


using namespace std;

class ObsAct: public StateObsAct {
    protected:
        string vname;

    public:
        void setVName(string str);

        string getVName() const;

        std::ostream& write(std::ostream& out);
};

#endif
