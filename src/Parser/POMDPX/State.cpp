// written by png shao wei
#include <string>
#include "State.h"

void State::setVNamePrev(string str) {
    vnamePrev = str;
}

void State::setVNameCurr(string str) {
    vnameCurr = str;
}

void State::setObserved(bool o) {
    observed = o;
}

const string State::getVNamePrev() const {
    return vnamePrev;
}

const string State::getVNameCurr() const {
    return vnameCurr;
}

const bool State::getObserved() const {
    return observed;
}
/*
   void State::setStateFunction(StateFunction* s) {
   sf = s;
   }

   StateFunction* State:: getStateFunction() {
   return sf;
   }*/

std::ostream& State::write(std::ostream& out) {
    out << "vnamePrev :" << vnamePrev << endl;
    out << "vnameCurr :" << vnameCurr << endl;
    out << "observation :" << observed << endl;
    out << "ValueEnum: " << endl;

    for (unsigned int i = 0; i < valueEnum.size(); i++) {
        out << valueEnum[i] << ".";
    }
    return out;
    //copy(valueEnum.begin(), valueEnum.end(), ostream_iterator<string>(out, ", "));


}
