// written by png shao wei

#include "ObsAct.h"
#include <iostream>
#include <string>
using namespace std;

void ObsAct::setVName(string str) {
    vname = str;
}

string ObsAct::getVName() const{
    return vname;
}

std::ostream& ObsAct::write(std::ostream& out) 
{
    out << "vname :" << vname << endl;
    out << "ValueEnum: " << endl;

    for (unsigned int i = 0; i < valueEnum.size(); i++) {
        out << valueEnum[i] << ".";
    }
    return out;
}

