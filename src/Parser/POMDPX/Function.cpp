// written by png shao wei
#include <algorithm>
#include "Function.h"

Function::Function()
{
}
void Function::setVNameCurr(string soa) {
    vnameCurr = soa;
}

void Function::setParents(vector<string> p) {
    parents = p;
}

string Function::getVNameCurr() const{
    return vnameCurr;
}

vector<string> Function::getParents() const{
    return parents;
}

void Function::initSparseTables(map<string, StateObsAct*>* mymap) {
    //if (parents.size() == 0)
    //sparseT->numCIValues.push_back(-999);
    vector<string> cIheader, uIheader;
    vector<int> numCIValues, numUIValues;
    for (unsigned int i = 0; i < parents.size(); i++) {
        cIheader.push_back(parents[i]);
        StateObsAct* soa = (*mymap)[parents[i]];
        if (soa != NULL)
            numCIValues.push_back(soa->getValueEnum().size());
        else
            numCIValues.push_back(1);
    }
    uIheader.push_back(vnameCurr);
    StateObsAct* soa = (*mymap)[vnameCurr];
    numUIValues.push_back(soa->getValueEnum().size());

    sparseT = SharedPointer<SparseTable>(new SparseTable(cIheader, uIheader, numCIValues, numUIValues));
}


void Function::fillTables(map<string, StateObsAct*>& mymap,
        const vector<string>& insttokens, const vector<double>& probttokensdouble) {
    bool simple = true;
    bool seestar = false;

    // this method should stop if a '-' or '*' is found.
    // in this case, it will call filLTables with the new parameters
    for (unsigned int i = 0; i < insttokens.size() && !seestar; i++) {

        if (insttokens[i] == "*" || insttokens[i] == "-") {
            seestar = true;
            simple = false;
            vector<string> allCombinations;
            if (i != insttokens.size() - 1) {
                if (mymap.find(parents[i]) != mymap.end()) {
                    allCombinations = mymap[parents[i]]->getValueEnum(); // get all the various enumerations
                } else {
                    cerr<<"Couldn't find mapping for:"<<parents[i]<<endl;
                    cerr<<"Please check XML file again."<<endl;
                    exit(-1);
                }
            } else {
                if (mymap.find(vnameCurr) != mymap.end()) {
                    allCombinations = mymap[vnameCurr]->getValueEnum();
                } else {
                    cerr<<"Couldn't find mapping for:"<<vnameCurr<<endl;
                    cerr<<"Please check XML file again."<<endl;
                    exit(-1);
                }
            }//end of if

            // multiple fillTables will be called, populating the table with entries
            for (unsigned int j = 0; j < allCombinations.size(); j++) {

                // replace an '-' or '*' with a string
                vector<string> temp = insttokens;
                string s(allCombinations[j]);
                temp.erase(temp.begin() + i);
                temp.insert(temp.begin() + i, s);

                // split the probtables if the input is a -
                // recursive calls
                if (insttokens[i] == "-") {
                    // splitTable will allocate the probability tokens to the correct parent
                    // note that one parent may still has more than one probability token
                    // assuming parents = action, X, parentA, parentB,
                    // and parentA has 2 elements a1,a2, while parentB has two elements,b1,2
                    // <Instance>action x - -</Instance>
                    // <ProbTable> x1 x2 x3 x4 </ProbTable>
                    // in this case,
                    // for the first iteration, when our program detects the first '-'
                    // x1 and x2 are allocated to the first element of the first '-', namely a1
                    // while x3 and x4 are allocated to the second element of the first '-,', namely a2


                    vector<double> st = splitTable(probttokensdouble, allCombinations.size(), j);
                    fillTables(mymap, temp, st);
                } else {
                    fillTables(mymap, temp, probttokensdouble);
                }
            }//end of for
        }//end of if
    }//end of for

    if (probttokensdouble.size() > 1)
        simple = false;

    //Base Case
    if (simple) {
        // simple means without * or -
        // separate the insttokens into the "tokens of parents" and "itself"
        // pass to simpleInsert
        string myself = insttokens.back();

        // Note new_insttokens size is now 1 less than insttokens
        vector<string> new_insttokens;
        new_insttokens.resize(insttokens.size() - 1);
        copy(insttokens.begin(), insttokens.end()-1, new_insttokens.begin());

        simpleSparseInsert(mymap, new_insttokens, myself, probttokensdouble.front());
    }
}

vector<double> Function::splitTable(const vector<double>& probttokensdouble, int size,
        int position) {
    vector<double> st;

    int factor = probttokensdouble.size() / size;

    for (int i = position * factor; i < position * factor + factor; i++) {
        st.push_back(probttokensdouble[i]);
    }
    return st;
}

void Function::simpleSparseInsert(map<string, StateObsAct*>& mymap, const vector<
        string>& insttokens, const string& myself, double prob) {

    SparseEntry st;
    int position = 0;

    vector<int> ci;
    for (unsigned int i = 0; i < insttokens.size(); i++) {
        position = mymap[parents[i]]->getPosition(insttokens[i]);
        ci.push_back(position);
    }

    UniqueIndex ui;
    ui.index = mymap[vnameCurr]->getPosition(myself);
    ui.value = prob;
    st.uniqueIndex.push_back(ui);

    sparseT->add(ci, st);
}

bool Function::checkNoMissingEntries(map<string, StateObsAct*> mymap, string& info) {
     stringstream ssinfo;
     vector<int> commonIndex;
     if (!sparseT->checkNoMissingEntries(commonIndex)) 
     {
	  ssinfo << "  Transition not specified for instance ";
	  for(int i=0;i<commonIndex.size();i++){
	       if(getParents()[i]!="null"){
		    StateObsAct* soa = mymap[getParents()[i]];
		    ssinfo << soa->getValueEnum()[commonIndex[i]] << " ";
	       }else{
		    ssinfo << "null" << endl;
	       }
	  }
	  ssinfo << endl;
	  info = ssinfo.str();
	  return false;
     }
     else
	  return true;
}//end of checkNoMissingEntries(map<string, StateObsAct*> mymap)


std::ostream& Function::write(std::ostream& out) {
    out << "\nvnameCurr :" << vnameCurr << endl;
    out << "parents: " << endl;

    for (unsigned int i = 0; i < parents.size(); i++) {
        out << parents[i] << ",";
    }
    sparseT->write(out);
    return out;
}
