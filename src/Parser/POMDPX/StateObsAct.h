// written by png shao wei
// StateObsAct is for base class
// ObsAct is for Observation and Action
// State inherits StateObsAct and it has additional previous and current values

#ifndef StateObsAct_H
#define StateObsAct_H

#include <iostream>
#include <vector>
#include <map>
#include <iterator>
using namespace std;

class StateObsAct {

    protected:
        vector<string> valueEnum;
        map<string, int> positionLookup;
        void preparePositionLookup();

    public:
        void print();
        void setValueEnum(const vector<string>& ve);
        int getPosition(const string& name);
        const vector<string>& getValueEnum() const;
        const bool containsInstance(const string& instanceName);
};

#endif


