// written by png shao wei

#include "StateObsAct.h"
#include <string>
#include <cassert>
using namespace std;

const vector<string>& StateObsAct::getValueEnum() const {
    return valueEnum;
}

void StateObsAct::setValueEnum(const vector<string>& ve) {
    valueEnum = ve;
    positionLookup.clear();
    return;
}

void StateObsAct::preparePositionLookup() {
    if (positionLookup.empty()) {
        for (int i=0; i<valueEnum.size(); i++) {
            positionLookup[valueEnum[i]] = i;
        }
    }
    return;
}

int StateObsAct::getPosition(const string& name) {
    preparePositionLookup();
    assert(containsInstance(name));
    return positionLookup[name];
}

void StateObsAct::print() {
    for (unsigned int i = 0; i < valueEnum.size(); i++) {
        cout << "i: " << i << " " << valueEnum[i] << endl;
    }
    cout << endl;
}

const bool StateObsAct::containsInstance(const string& instanceName) {
    preparePositionLookup();
    return positionLookup.find(instanceName) != positionLookup.end();
}
