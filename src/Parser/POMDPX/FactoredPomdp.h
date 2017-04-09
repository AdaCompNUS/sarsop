// written by png shao wei
#ifndef FactoredPomdp_H
#define FactoredPomdp_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <ostream>
#include "tinyxml.h"
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <stdlib.h>
#include <map>
#include <set>
#include <iomanip>

#include "CPTimer.h"
#include <signal.h>
#include "MathLib.h"
#include "State.h"
#include "ObsAct.h"
//#include "StateFunction.h"
#include "PreCMatrix.h"
#include "POMDPLayer.h"
#include "Function.h"
#include "SparseTable.h"
#include "MObject.h"
#define REWVALUEENUM "rew"

/* #define FULLY_OBSERVED 0 */
/* #define FULLY_UNOBSERVED 1 */
/* #define MIXED 2 */
/* #define MIXED_REPARAM 3 */

#define CLEAN_EXIT 0
#define XML_INPUT_ERROR -1

#ifdef DEBUG_LOG_ON
// the following definitions are for debugging purposes
// this is for printing the contents of the xml file. it helps to debug the parser
#define DEBUGREADXMLINPUT 1
// this is for printing the OLD POMDP format after conversion
#define DEBUGFASTCONVERSIONSTATE 1
#define DEBUGFASTCONVERSIONOBS 1
#define DEBUGFASTCONVERSIONREWARD 1
#define DEBUGFASTCONVERSIONOTHERS 1
#define DEBUGFASTCONVERSIONBELIEF 1
// this is for printing the Factored POMDP format after conversion
#define DEBUGFACTOREDCONVERSIONSTATE 1
#define DEBUGFACTOREDCONVERSIONOBS 1
#define DEBUGFACTOREDCONVERSIONREWARD 1
#define DEBUGFACTOREDCONVERSIONTERMINAL 1
#define DEBUGFACTOREDCONVERSIONOTHERS 1
#define DEBUGFACTOREDCONVERSIONBELIEF 1
#else
#define DEBUGREADXMLINPUT 0

#define DEBUGFASTCONVERSIONSTATE 0
#define DEBUGFASTCONVERSIONOBS 0
#define DEBUGFASTCONVERSIONREWARD 0
#define DEBUGFASTCONVERSIONOTHERS 0
#define DEBUGFASTCONVERSIONBELIEF 0

#define DEBUGFACTOREDCONVERSIONSTATE 0
#define DEBUGFACTOREDCONVERSIONOBS 0
#define DEBUGFACTOREDCONVERSIONREWARD 0
#define DEBUGFACTOREDCONVERSIONTERMINAL 0
#define DEBUGFACTOREDCONVERSIONOTHERS 0
#define DEBUGFACTOREDCONVERSIONBELIEF 0
#endif

using std::cin;
using std::cout;
using std::endl;

typedef vector<vector<PreSparseMatrix> > vvPreSparseMatrix;
typedef vector<vector<SharedPointer<SparseMatrix> > > vvSparseMatrix;

namespace momdp {

    class FactoredPomdp {
        friend class MOMDP;

        private:
            //////////////START: DATA STRUCTURES////////////////////////

            string filename;

            double discount;
            vector<State> stateList;
            vector<ObsAct> observationList;
            vector<ObsAct> actionList;
            vector<ObsAct> rewardList;
            vector<ObsAct> terminalStateRewardList;

            map<string, StateObsAct*> mymap;
            vector<Function> stateFunctionList;
            vector<Function> beliefFunctionList;
            vector<Function> processedBeliefFunctionList;
            vector<Function> observFunctionList;
            vector<Function> actionFunctionList;
            vector<Function> rewardFunctionList;
            vector<Function> terminalStateRewardFunctionList;

            map<string, Function*> mapFunc;
            vector<string> canonicalNamePrev;
            vector<string> canonicalNameCurr;
            vector<string> canonicalNameForTerminal;

            /////////////

            vector<vector<vector<double> > > oldStateTransition; // contains a vector of actions, each such vector contains a 2d array
            vector<vector<vector<double> > > oldObservTransition;
            vector<vector<double> > oldRewardTransition;

        public:

            // for sylvie
            POMDPLayer layer;

            //////////////////////////////////
            ////start of method
            ////////////////////////////////

            FactoredPomdp();
            FactoredPomdp(string f);
            ~FactoredPomdp();

            void Tokenize(const string& str, vector<string>& tokens,
                    const string& delimiters = " ");

            State createState(TiXmlElement* varChild);
            ObsAct createObservation(TiXmlElement* varChild);
            ObsAct createAction(TiXmlElement* varChild);

            void createInitialBelief(TiXmlElement* varChild);
            const bool checkStateNameExists(string stateName) const;
            const bool checkActionNameExists(string actionName) const;
            const bool checkObsNameExists(string obsName) const;
            const bool checkRewardNameExists(string rewardName) const;
            const bool checkTerminalNameExists(string rewardName) const;
            const bool checkParentNameExists(string parentName) const;
            const bool checkInstanceMatchesParent(string instanceName, string parent);
            const State& findState(string varName);
            const bool checkIdentityIsValid(vector<string> tokens) const;
	    bool checkFunctionProbabilities(Function* f, TiXmlElement* xmlnode, string whichFunction);
            Function createFunction(TiXmlElement* pFunction, int whichFunction);
	    void printXMLErrorHeader(TiXmlBase* base);
	    void printXMLWarningHeader(TiXmlBase* base);
            //StateFunction createStateFunction(TiXmlElement* pStructFunction);


            const bool checkRewardFunctionHasOnlyPreviousTimeSliceAndAction() const;
	    const set<string> getRewardFunctionCurrentTimeSliceVars(Function* rewardFunc) ;
            SharedPointer<SparseTable> removeRedundantUIsFromReward(SharedPointer<SparseTable> st);
            SharedPointer<SparseTable> combineSimilarEntriesInReward(SharedPointer<SparseTable> st);
            SharedPointer<SparseTable> preprocessRewardTable(); 	//called by convertFast
            void preprocessRewardFunction();	//called by convertFactored


            // reading in the file
            unsigned int start();
            const int checkProblemType();

            // rearrange states so that observed states will come before unobserved states
            // this is necessary because the simulator makes this assumption
            void sortStateList();

            SharedPointer<SparseTable>  finalRewardTable;
            SharedPointer<SparseTable>  finalStateTable;
            SharedPointer<SparseTable>  finalBeliefTable;
            SharedPointer<SparseTable>  mergeTables(vector<Function>* functionList, int whichFunction, ofstream& debugfile, bool printDebugFile);

	    SharedPointer<SparseTable>  mergeSparseTables(vector<SharedPointer<SparseTable> > stList, int whichFunction, ofstream& debugfile, bool printDebugFile); 
            void preprocessBeliefTables(ofstream& debugfile, bool printDebugFile);    
	    bool preprocessBeliefTablesDone;
            void mergeBeliefTables(ofstream& debugfile, bool printDebugFile);


            const void defineCanonicalNames();

            SharedPointer<SparseTable> expandObsRewSparseTable(SharedPointer<SparseTable> st, int whichFunction);

            bool validateModel(Function sf, string& info);
            const bool isPreviousTimeSlice(string name) const;
            const bool isCurrentTimeSlice(string name) const;
            // conversion to the old POMDP
            // in the sequence, do not change sequence


            map<string, int> actionStringIndexMap;
            int numActions;
            void mapActionsToValue();

            map<string, int> observationStringIndexMap;
            int numObservations;
            void mapObservationsToValue();
            map<int, int> observationUIIndexMap;
            void mapObservationsUIsToValue(SharedPointer<SparseTable> st);

            map<string, int> positionStringIndexMap;

            map<int, int> fastPositionCIIndexMap;
            map<int, int> fastPositionUIIndexMap;

	    map<string, int> getStartXYVarValues();
	    map<string, int> getStartActionXYVarValues();
	    bool getNextActionXYVarValues(map<string, int> &curValues, int &action, int &stateX, int& stateY);
	    bool getNextXYVarValues(map<string, int> &curValues,int &stateX, int &stateY);
	    bool getNextActionXXpYVarValues(map<string, int> &curValues, int &action, int &stateX, int &stateXp, int &stateY);
	    map<string, int> getStartSVarValues();
	    map<string, int> getStartActionSVarValues();
	    bool getNextActionSVarValues(map<string, int> &curValues, int &action, int &stateNum);
	    bool getNextSVarValues(map<string, int> &curValues,int &stateNum);

            int numMergedStates;
            void resortFastStateTables(ofstream& debugfile, bool printDebugFile);
            void mapFastStatesToValue();
            void mapFastIndexesToValues(SharedPointer<SparseTable> st);
            void convertFast();
            void convertFastStateTrans();
            void convertFastObsTrans();
            void convertFastNoObservationsVariables();
            void convertFastRewardTrans();
            void convertFastBelief();

            void convertFastVariables();
            SharedPointer<SparseTable> reduceUnmatchedCIWithUI(SharedPointer<SparseTable>  st, ofstream& debugfile, bool printDebugFile);

            map<string, int> positionXStringIndexMap;
            map<string, int> positionYStringIndexMap;

            // factored conversion to the Factored POMDP
            // in the sequence, do not change sequence
            map<int, int> factoredPositionCIIndexMap;
            map<int, int> factoredPositionUIIndexMap;
            int numMergedStatesX;
            int numMergedStatesY;

	    vector<vector<PreSparseMatrix> > createVvPreSparseMatrix(int a, int b, int c, int d);
	    void printSparseMatrix(string title, vector<vector<SharedPointer<SparseMatrix> > > M, ofstream& debugfile);

            void resortFactoredStateTables(ofstream& debugfile, bool printDebugFile, const int MIXEDTYPE);
            void mapFactoredStatesToValue();
            void mapFactoredCIsToValue(SharedPointer<SparseTable> st);
            void mapFactoredStateUIsToValue(SharedPointer<SparseTable> st);
            void mapFactoredBeliefIndexesToValue(SharedPointer<SparseTable> st);
            void convertFactored();
            void convertFactoredReparam();
            void convertFactoredStateTrans();
            void convertFactoredStateReparamTrans();
            void expandFactoredStateTable(SharedPointer<SparseTable> sf);
            void convertFactoredObsTrans();
            void convertFactoredNoObservationsVariables();
            void convertFactoredRewardTrans();
            void convertFactoredTerminalStateReward();

            void convertFactoredBeliefCommon(ofstream& debugfile, bool printDebugFile);
            void convertFactoredBelief();
            void convertFactoredBeliefReparam();
            void convertFactoredVariables();

            // a method for converting data structres in PreSparseMatrix to the SparseMatrix in sla.h
            vector<vector<SharedPointer<SparseMatrix> > > helperPreSparseMatrixToSparseMatrix(vector< vector<PreSparseMatrix> > precm);
            vector<vector<vector<SharedPointer<SparseMatrix> > > > helperPreSparseMatrixToSparseMatrix(vector< vector<vector<PreSparseMatrix> > >precm);

            //symbolic information
            map<string, string> getFactoredObservedStatesSymbols(int stateNum);
            map<string, string> getFactoredUnobservedStatesSymbols(int stateNum);
            map<string, string> getActionsSymbols(int actionNum);
            map<string, string> getObservationsSymbols(int observationNum);


    };//end of class
}//end of namespace

#endif
