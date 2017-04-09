
//
// taken over and maintained by Won Kok Sung since Dec 2008.
// taken over by png shao wei since 4th March 2009

#include "FactoredPomdp.h"

using namespace momdp;

struct EndState{
    int endStateX;
    int endStateY;
    double probX;
    double probY;
} typedef EndState;

struct IndexProbTuple{
    int index;
    double prob;
} typedef IndexProbTuple;

FactoredPomdp::FactoredPomdp() 
{
    preprocessBeliefTablesDone = false;
}

FactoredPomdp::FactoredPomdp(string f){
    preprocessBeliefTablesDone = false;
    filename = f;
}

FactoredPomdp::~FactoredPomdp() {
    DEBUG_LOG (cout << "destroying FactoredPomdp" << endl;);


    DEBUG_LOG (cout << "FactoredPomdp destroyed" << endl; );
}

/////////////////////////////////
// USEFUL GENERIC METHODS
// TOKENIZER
void FactoredPomdp::Tokenize(const string& str, vector<string>& tokens, const string& delimiters) 
{

    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos) 
    {
	// Found a token, add it to the vector.
	tokens.push_back(str.substr(lastPos, pos - lastPos));
	// Skip delimiters.  Note the "not_of"
	lastPos = str.find_first_not_of(delimiters, pos);
	// Find next "non-delimiter"
	pos = str.find_first_of(delimiters, lastPos);
    }
}
//End of Tokenizer
//////////////////////////////////

//////////////////////////////////
//////////////////////////////////////////////////
//Useful Methods For Variables///////////////////

State FactoredPomdp::createState(TiXmlElement* varChild) 
{

    string vnamePrev = varChild->Attribute("vnamePrev");
    string vnameCurr = varChild->Attribute("vnameCurr");
    const char* fullyObserved;

    if (varChild->Attribute("fullyObs"))
	fullyObserved = varChild->Attribute("fullyObs");
    else
	fullyObserved = "false"; //default is false


    // This is to handle ValueEnum and ValueConcise cases
    string valueEnumList = "";
    const char* valueConciseNum = NULL;

    State newState;
    newState.setVNamePrev(vnamePrev);
    newState.setVNameCurr(vnameCurr);
    if (strcmp(fullyObserved,"true")==0)
	newState.setObserved(true);
    else
	newState.setObserved(false);
    vector<string> tokens;

    if (varChild->FirstChild("ValueEnum")) {
	valueEnumList = varChild->FirstChild("ValueEnum")->ToElement()->GetText();
	Tokenize(valueEnumList, tokens);
    } else {
	valueConciseNum = varChild->FirstChild("NumValues")->ToElement()->GetText();
	int numOfStates = atoi(valueConciseNum);

	for (int i = 0; i < numOfStates; i++) {
	    char buffer[10]; //Not expecting the number of states to be more than 10 digits
	    sprintf(buffer, "s%d", i); //We require all states to be prefixed with 's'
	    tokens.push_back(buffer);
	}
    }
    newState.setValueEnum(tokens);

    return newState;
}//end of createState

ObsAct FactoredPomdp::createObservation(TiXmlElement* varChild) {

    ObsAct obs;

    string vname = varChild->Attribute("vname");
    string valueEnumList = "";
    const char* valueConciseNum = NULL;

    obs.setVName(vname);
    vector<string> tokens;
    if (varChild->FirstChild("ValueEnum")) {
	valueEnumList
	    = varChild->FirstChild("ValueEnum")->ToElement()->GetText();
	Tokenize(valueEnumList, tokens);
    } else {
	valueConciseNum
	    = varChild->FirstChild("NumValues")->ToElement()->GetText();
	int numOfObs = atoi(valueConciseNum);
	for (int i = 0; i < numOfObs; i++) {
	    char buffer[10]; //Not expecting the number of observations to be more than 10 digits
	    sprintf(buffer, "o%d", i); //We require all obs to be prefixed with 'o'
	    tokens.push_back(buffer);
	}
    }

    obs.setValueEnum(tokens);

    return obs;
}//end of create observation

ObsAct FactoredPomdp::createAction(TiXmlElement* varChild) {

    ObsAct act;

    string vname = varChild->Attribute("vname");

    act.setVName(vname);
    vector<string> tokens;
    string valueEnumList = "";
    const char* valueConciseNum = NULL;
    if (varChild->FirstChild("ValueEnum")) {
	valueEnumList
	    = varChild->FirstChild("ValueEnum")->ToElement()->GetText();
	Tokenize(valueEnumList, tokens);
    } else {
	valueConciseNum
	    = varChild->FirstChild("NumValues")->ToElement()->GetText();
	int numOfAct = atoi(valueConciseNum);
	for (int i = 0; i < numOfAct; i++) {
	    char buffer[10]; //Not expecting the number of actions to be more than 10 digits
	    sprintf(buffer, "a%d", i); //We require all actions to be prefixed with 'a'
	    tokens.push_back(buffer);
	}
    }

    act.setValueEnum(tokens);

    return act;
}//end of createAction

// END OF Useful Methods For Variables////////
//////////////////////////////////////////////


//////////////////////////////////////////////
////////Useful Methods For Functions //////

// check the <instance> entry and returns true if the last 2 tokens are '-'
// this is the only situation where 'identity' can be used
const bool FactoredPomdp::checkIdentityIsValid(vector<string> tokens) const {
    bool valid = true;
    int numDash = 0;
    for (unsigned int i=0; i < tokens.size()-1; i++) {
	if (strcmp((tokens.at(i).c_str()),"-") == 0)
	    numDash++;
    }
    if (numDash != 1) valid = false;
    if (strcmp((tokens.at(tokens.size()-1).c_str()),"-") != 0)
	valid = false;
    return valid;
}

const bool FactoredPomdp::checkStateNameExists(string stateName) const {
    for (unsigned int i=0; i < stateList.size(); i++) {
	if (stateList[i].getVNamePrev() == stateName || stateList[i].getVNameCurr() == stateName)
	    return true;
    }
    return false;
}

const bool FactoredPomdp::checkObsNameExists(string obsName) const{
    for (unsigned int i=0; i < observationList.size(); i++) {
	if (observationList[i].getVName() == obsName)
	    return true;
    }
    return false;
}

const bool FactoredPomdp::checkActionNameExists(string actionName) const{
    for (unsigned int i=0; i < actionList.size(); i++) {
	if (actionList[i].getVName() == actionName)
	    return true;
    }
    return false;
}

const bool FactoredPomdp::checkRewardNameExists(string rewardName) const{
    for (unsigned int i=0; i < rewardList.size(); i++) {
	if (rewardList[i].getVName() == rewardName)
	    return true;
    }
    return false;
}

const bool FactoredPomdp::checkTerminalNameExists(string terminalName) const{
    for (unsigned int i=0; i < terminalStateRewardList.size(); i++) {
	if (terminalStateRewardList[i].getVName() == terminalName)
	    return true;
    }
    return false;
}

    const bool FactoredPomdp::checkParentNameExists(string parentName) const{
	if (checkStateNameExists(parentName) || (checkActionNameExists(parentName) || (checkObsNameExists(parentName))))
	    return true;
	else
	    return false;
    }

const bool FactoredPomdp::checkInstanceMatchesParent(string instanceName, string parent) {
    StateObsAct* soa = mymap[parent];
    if (soa->containsInstance(instanceName)) return true;
    else return false;
}


const State& FactoredPomdp::findState(string varName) {
    for (unsigned int i=0; i < stateList.size(); i++) {
	if (stateList[i].getVNamePrev() == varName || stateList[i].getVNameCurr() == varName)
	    return stateList[i];
    }
    cerr << "State named \" " << varName << "\"cannot be found" << endl;
    exit(XML_INPUT_ERROR);
}

void FactoredPomdp::printXMLErrorHeader(TiXmlBase* base){
    cerr << "ERROR" <<endl;
    cerr << "  " << filename <<":Line " << base->Row() <<":" << endl << "  ";
}

void FactoredPomdp::printXMLWarningHeader(TiXmlBase* base){
    cerr << "WARNING" <<endl;
    cerr << "  " << filename <<":Line " << base->Row() <<":"<< endl << "  ";
}

Function FactoredPomdp::createFunction(TiXmlElement* pFunction, int whichFunction) 
{
    // note that state function, observation function, and reward function have slighly different structure
    const char* pvtable;
    const char* pvname;
    if (whichFunction == REWARDFUNCTION || whichFunction == TERMINALFUNCTION) {
	pvtable = "ValueTable";
    } else
	pvtable = "ProbTable";

    //Handles the <Var> tag
    Function nFunction;

    string vnameCurr = pFunction->FirstChild("Var")->ToElement()->GetText();

    // check if there is error in <Var>
    switch (whichFunction) 
    {

	case BELIEFFUNCTION:
	case STATEFUNCTION:
	    if (!(checkStateNameExists(vnameCurr))) {
		printXMLErrorHeader(pFunction->FirstChild("Var"));
		cerr << "In State Function: " << vnameCurr << " has not been declared. \n  Check your pomdpX file for missing declaration within <Variable> tag." << endl;
		exit(XML_INPUT_ERROR);
	    }
	    break;

	case OBSERVFUNCTION:
	    if (!(checkObsNameExists(vnameCurr))) {
		printXMLErrorHeader(pFunction->FirstChild("Var"));
		cerr << "In Observation Function: " << vnameCurr << " has not been declared. \n  Check your pomdpX file for missing declaration within <Variable> tag." << endl;
		exit(XML_INPUT_ERROR);
	    }
	    break;
	case REWARDFUNCTION:

	    if (!(checkRewardNameExists(vnameCurr))) {
		printXMLErrorHeader(pFunction->FirstChild("Var"));
		cerr << "In Reward Function: " << vnameCurr << " has not been declared. \n  Check your pomdpX file for missing declaration within <Variable> tag." << endl;
		exit(XML_INPUT_ERROR);
	    }
	    break;

	case TERMINALFUNCTION:

	    if (!(checkTerminalNameExists(vnameCurr))) {
		printXMLErrorHeader(pFunction->FirstChild("Var"));
		cerr << "In Terminal State Reward Function: " << vnameCurr << " has not been declared. \n  Check your pomdpX file for missing declaration within <Variable> tag." << endl;
		exit(XML_INPUT_ERROR);
	    }
	    break;
    }//end of switch

    nFunction.setVNameCurr(vnameCurr);

    //Handles the <Parent> tag
    vector<string> parentTokens;
    Tokenize(pFunction->FirstChild("Parent")->ToElement()->GetText(), parentTokens);
    //Check if the parents input are valid
    for (int i=0; i < parentTokens.size(); i++) {
	if (!(checkParentNameExists(parentTokens[i]))) {
	    if (!((parentTokens[i] == "null") && (whichFunction == BELIEFFUNCTION))) {
		printXMLErrorHeader(pFunction->FirstChild("Parent"));
		cerr << "In Parents, " << parentTokens[i] <<" has not been declared.\n  Check your pomdpX file for missing declaration or invalid parent name" << endl;
		exit(XML_INPUT_ERROR);
	    }
	}
    }

    nFunction.setParents(parentTokens);
    // create sparse version
    nFunction.initSparseTables(&mymap);


    // fill up the tables
    TiXmlElement *pInstance = pFunction->FirstChild("Parameter")->ToElement();

    string paramType = "";
    if (pInstance->Attribute("type") == NULL) paramType = "TBL"; //the default case
    else paramType = pInstance->Attribute("type");
    if (paramType == "DD") {
	cerr<<"Sorry the parameter type=\"DD\" is currently not implemented."<<endl;
	cerr<<"Please use type=\"TBL\" instead"<<endl;
	exit(XML_INPUT_ERROR);
    }

    TiXmlElement *pNextSibling = pInstance->FirstChildElement("Entry");
    //TODO better error report when no 'Entry' tag given?
    while (pNextSibling != 0) { //Looping thru the <Entry> elements
	string instance = pNextSibling->FirstChildElement("Instance")->GetText();
	TiXmlElement *pInstance = pNextSibling->FirstChildElement("Instance");
	vector<string> insttokens;
	Tokenize(instance, insttokens);

	// first, check if the number of parameters in <Instance> is matches the number of parents
	// the number for REWARDFUNCTION is exactly the same but it has an additional one 
	// for STATEFUNCTION AND OBSERVATION FUNCTION
	// second, check if <Instance> contains correct input corresponding to the parents
	switch (whichFunction) 
	{
	    case OBSERVFUNCTION:
		for (unsigned int i=0; i < parentTokens.size(); i++) {
		    if (!(checkActionNameExists(parentTokens[i]))) {
			if (isPreviousTimeSlice(parentTokens[i])) {
			    printXMLErrorHeader(pFunction->FirstChild("Parent"));
			    cerr << "In Observation Function, all the parents must be of the next time slice." << endl;
			    exit(XML_INPUT_ERROR);
			}
		    }
		}
	    case STATEFUNCTION:

		if (parentTokens.size() + 1 != insttokens.size()) {
		    printXMLErrorHeader(pInstance);
		    cerr<<"In Function " << vnameCurr << endl;
		    cerr<<"  instance " << instance << " does not match the parents" << endl;
		    cerr<<"  Check if number of entries in the <Instance> tag is correct" << endl;
		    exit(XML_INPUT_ERROR);
		}

		// check the input in <Instance> except for the last one
		for (unsigned int i=0; i<parentTokens.size(); i++) {
		    if (insttokens[i] != "*" && insttokens[i] != "-") {
			if (!checkInstanceMatchesParent(insttokens[i],parentTokens[i])) {
			    printXMLErrorHeader(pInstance);
			    cerr<<"In Function: " << vnameCurr << endl;
			    cerr << "  in <Instance> ";
			    for (unsigned int j=0; j < insttokens.size(); j++)
				cerr << insttokens[j] << " ";
			    cerr<< "\n  "<< insttokens[i] << " does not exist "<< endl;
			    exit(XML_INPUT_ERROR);
			}//end of if
		    }//end of if
		}//end of for

		// check the last input in <Instance> which corresponds to vnameCurr
		if (insttokens[insttokens.size()-1] != "*" && insttokens[insttokens.size()-1] != "-") 
		{
		    if (!checkInstanceMatchesParent(insttokens[insttokens.size() -1],vnameCurr)) 
		    {
			printXMLErrorHeader(pInstance);
			cerr<<"In Function: " << vnameCurr << endl;
			cerr << "  in <Instance> ";
			for (unsigned int j=0; j < insttokens.size(); j++)
			    cerr << insttokens[j] << " ";
			cerr << "\n  " <<  insttokens[insttokens.size()-1] << " does not exist "<< endl;
			exit(XML_INPUT_ERROR);
		    }
		}//end of if

		break;

	    case TERMINALFUNCTION:
	    case REWARDFUNCTION:
		if (parentTokens.size() != insttokens.size()) {
		    printXMLErrorHeader(pInstance);
		    cerr<<"In Function: " << vnameCurr << endl;
		    cerr<<"  " << instance << " does not match the parents" << endl;
		    cerr<<"  Check if number of entries in the <Instance> tag is correct" << endl;
		    exit(XML_INPUT_ERROR);
		}

		for (unsigned int i=0; i<parentTokens.size(); i++) {
		    if (insttokens[i] != "*" && insttokens[i] != "-") {
			if (!checkInstanceMatchesParent(insttokens[i],parentTokens[i])) {
			    printXMLErrorHeader(pInstance);
			    cerr<<"In Function: " << vnameCurr << endl;
			    cerr << "  in <Instance> ";
			    for (unsigned int j=0; j < insttokens.size(); j++)
				cerr << insttokens[j] << " ";
			    cerr<< "\n  "<< insttokens[i] << " does not exist "<< endl;
			    exit(XML_INPUT_ERROR);
			}
		    }
		}//end of for
		break;


	    case BELIEFFUNCTION:
		if (parentTokens.size() == 1 && parentTokens[0] == "null") {

		    if (parentTokens.size() != insttokens.size()) {
			printXMLErrorHeader(pInstance);
			cerr<<"In Belief Function " << vnameCurr << endl;
			cerr<<"  Instance " << instance << " does not match the parents" << endl;
			cerr<<"  Check if the <Instance> input is correct" << endl;
			exit(XML_INPUT_ERROR);
		    }

		    /*if (!(insttokens.size() == 1 && insttokens[0] == "-")) {
		      printXMLErrorHeader(pInstance);
		      cerr<<"In Belief Function " << vnameCurr << endl;
		      cerr << "  In Belief Function, if the parent is null, the Instance must be -"<<endl;
		      exit(XML_INPUT_ERROR);
		      }*/

		}

		break;


	}//end of switch

    TiXmlElement *tableElement = pNextSibling->FirstChildElement(pvtable);
    if (tableElement == NULL) {
        cerr << "ERROR\n  Cannot find tag " << pvtable << endl;
        exit(XML_INPUT_ERROR);
    }
	string table = tableElement->GetText();
	vector<double> probttokensdouble;

	std::transform(table.begin(), table.end(),table.begin(), ::tolower);
	//check for identity keyword in <ProbTable>

	if (strcmp(table.c_str(),"uniform") == 0) {
	    if (whichFunction == BELIEFFUNCTION) {
		if (parentTokens[0] == "null" && insttokens[0] == "-") {
		    const State& s = findState(vnameCurr);
		    int valueEnum = s.getValueEnum().size();

		    for (int i = 0; i < valueEnum; i++) {
			probttokensdouble.push_back(1.0 / valueEnum);
		    }
		}else {
		    printXMLErrorHeader(pNextSibling->FirstChild(pvtable));
		    cerr << "Parents must be null and instance must be - for \"uniform\"" << endl;
		    exit(XML_INPUT_ERROR);
		}
	    }else {
		printXMLErrorHeader(pNextSibling->FirstChild(pvtable));
		cerr << "uniform keyword can only be used for initial belief" << endl;
		exit(XML_INPUT_ERROR);
	    }
	} else if (strcmp(table.c_str(),"identity") == 0) {

	    if (checkIdentityIsValid(insttokens)) {

		int indexDash = -99;
		for (unsigned int i=0; i < insttokens.size()-1; i++)
		    if (insttokens[i] == "-") indexDash = i;

		if (indexDash == -99) assert(false);

		const State& lastParent = findState(parentTokens[indexDash]);
		const State& self = findState(vnameCurr);

		if (lastParent.getValueEnum().size() == self.getValueEnum().size()) {
		    // push a identity matrix
		    for (unsigned int i=0; i < self.getValueEnum().size(); i++) {
			for (unsigned int j=0; j < self.getValueEnum().size(); j++) {
			    if (i==j)
				probttokensdouble.push_back(1);
			    else
				probttokensdouble.push_back(0);
			}
		    }
		}else {
		    printXMLErrorHeader(pNextSibling->FirstChild(pvtable));
		    cerr<<"In Function " << vnameCurr << endl;
		    cerr << "  The last Parent token should have the same number of enumerated states as the Var itself if 'identity' keyword is used" << endl;
		    exit(XML_INPUT_ERROR);
		}

	    }else {
		printXMLErrorHeader(pNextSibling->FirstChild(pvtable));
		cerr <<"In Function " << vnameCurr << endl;
		cerr << "  Error in using \"identity\" in Prob Table. Check whether <Instance> contains two '-' as the last variables" << endl;
		exit(XML_INPUT_ERROR);
	    }
	}else {

	    // tokenize the prob table 
	    vector<string> probttokens;
	    Tokenize(table, probttokens);


	    // if the tokens contain 'identity', the input file has a bug
	    for (unsigned int i=0; i < probttokens.size(); i++) {    
		if (probttokens[i] == "identity" || (probttokens[i] == "uniform")) {
		    printXMLErrorHeader(pNextSibling->FirstChild(pvtable));
		    cerr << "The keyword \"" << probttokens[i] << "\" must be used without other arguments\n";
		    exit(XML_INPUT_ERROR);
		}
	    }

	    int numEntries = 1;
	    int numTokens;
	    if (whichFunction == REWARDFUNCTION || whichFunction == TERMINALFUNCTION)
		numTokens = insttokens.size();
	    else
		numTokens = insttokens.size() -1;

	    for (unsigned int j=0 ; j < numTokens; j++) {
		if (insttokens[j] == "-") {
		    StateObsAct* soa = mymap[parentTokens[j]];
		    numEntries *= soa->getValueEnum().size();
		}
	    }

	    if (whichFunction != REWARDFUNCTION && whichFunction != TERMINALFUNCTION) {
		if (insttokens[insttokens.size()-1] == "-") {
		    StateObsAct* soa = mymap[vnameCurr];
		    numEntries *= soa->getValueEnum().size();
		}
	    }


	    if (numEntries != probttokens.size()) {
		printXMLErrorHeader(pNextSibling->FirstChild(pvtable));
		cerr << "In <Instance>";
		for (unsigned int k=0; k < insttokens.size(); k++) {
		    cerr << insttokens[k] << " ";
		}
		cerr << "</Instance>\n  <ProbTable>";
		for (unsigned int k=0; k < probttokens.size() ; k++) {
		    cerr << probttokens[k] << " ";
		}
		cerr << "</ProbTable>\n  ";
		cerr << "The ProbTable does not contain the correct number of entries."<< endl;
		exit(XML_INPUT_ERROR);
	    }
	    //End of else

	    for (unsigned int i = 0; i < probttokens.size(); i++) {
		probttokensdouble.push_back(atof(probttokens[i].c_str()));
	    }

	}
	// for reward function
	if (whichFunction == REWARDFUNCTION || whichFunction == TERMINALFUNCTION)
	    insttokens.push_back(REWVALUEENUM);


	nFunction.fillTables(mymap, insttokens, probttokensdouble);

	// next entry
	pNextSibling = pNextSibling->NextSiblingElement();
    }//end of while

    return nFunction;
}//end of create Function


//////End of Useful Methods for Functions//////
///////////////////////////////////// //////

//////////////////////////////////////////////
////////Useful Methods For StructParams//////


// END OF Useful Methods For StructParams//////
//////////////////////////////////////////////

unsigned int FactoredPomdp::start() {
    ofstream debugfile;
    if (DEBUGREADXMLINPUT) 
    {
	debugfile.open("debug_ReadXMLInput.txt");
    }

    // fstream inputFile;
    // inputFile.open(filename.c_str(), ios::in);
    // string line;
    // if (inputFile.is_open())
    // {
    //   while (! inputFile.eof() )
    //   {
    //     getline (inputFile,line);
    //     cout << line << endl;
    //   }
    //  inputFile.close();
    // }

    /////////////START: TINYXMLPARSER/////////////////////////

    TiXmlDocument doc(filename.c_str());

    TiXmlHandle hDoc(&doc); // the handler
    bool loadOkay = doc.LoadFile();

    if (!loadOkay) 
    {
	cerr << "ERROR\n  Could not load pomdpX file"<<endl ;
	cerr << "  Line"<< doc.ErrorRow() <<":"<< doc.ErrorDesc() << endl;
	cerr << "Check pomdpX file with pomdpX's XML schema using a XML validator." << endl;
	exit(1);
    }
    if (DEBUGREADXMLINPUT)
	debugfile << "** Docs read from " << filename.c_str() << endl;

    // printing out Value: pomdpx
    TiXmlElement *pElem = hDoc.FirstChildElement().ToElement();
    string msg = pElem->Value();
    cout << "input file   : " << filename << endl;

    // printing out the description of the file
    TiXmlElement *pElemDesc = hDoc.FirstChild("pomdpx").FirstChild("Description").ToElement();
    string description;
    if(pElemDesc)
	description = pElemDesc->GetText();
    if (DEBUGREADXMLINPUT)
	debugfile << "description: " << description << endl;

    // getting the discount of the file
    TiXmlElement *pElemDisc =
	hDoc.FirstChild("pomdpx").FirstChild("Discount").ToElement();
    if(!pElemDisc){
	cerr << "ERROR\n  Cannot find Discount tag" << endl;
	exit(1);
    }
    string disstr = pElemDisc->GetText();
    discount = atof(disstr.c_str());
    if (DEBUGREADXMLINPUT)
	debugfile << "discount: " << discount << endl;

    // getting the various children of variables
    TiXmlElement *pElemVariable = hDoc.FirstChild("pomdpx").FirstChild("Variable").ToElement();
    if(!pElemVariable){
	cerr << "ERROR\n  Cannot find Variable tag" << endl;
	exit(1);
    }

    TiXmlElement *pNextSibling = pElemVariable->FirstChild()->ToElement();
    while (pNextSibling != 0) {

	const char* tmp = pNextSibling->Value();
	string s = tmp;
	if (s == "StateVar") {
	    State nState = createState(pNextSibling);
	    stateList.push_back(nState); // this will create a new nState object into the statelist

	} else if (s == "ObsVar") { // currently the code only works for one observation
	    ObsAct obs = createObservation(pNextSibling);
	    observationList.push_back(obs);

	} else if (s == "ActionVar") { // currently the code only works for one action

	    ObsAct act = createAction(pNextSibling);
	    actionList.push_back(act);
	} else if (s == "RewardVar") {

	    string vname=pNextSibling->Attribute("vname");
	    ObsAct rew ;
	    rew.setVName(vname);

	    vector<string> rewardEnum;
	    rewardEnum.push_back(REWVALUEENUM);
	    rew.setValueEnum(rewardEnum);
	    rewardList.push_back(rew);

	} else if (s == "TerminalStateRewardVar") {

	    string vname=pNextSibling->Attribute("vname");
	    ObsAct rew ;
	    rew.setVName(vname);

	    vector<string> rewardEnum;
	    rewardEnum.push_back(REWVALUEENUM);
	    rew.setValueEnum(rewardEnum);
	    terminalStateRewardList.push_back(rew);

	}else {
	    printXMLErrorHeader(pNextSibling);
	    cerr << "  Unknown XML tag: " << s << " encountered" << endl;
	    exit(XML_INPUT_ERROR);
	}

	pNextSibling = pNextSibling->NextSiblingElement();

    }//end of while

    //// rearrange states so that observed states will come before unobserved states
    //// this is necessary because the simulator makes this assumption
    sortStateList();

    //////////////////////////////////////////////
    // printing the variables information
    if (DEBUGREADXMLINPUT)
	debugfile << "State List Size: " << stateList.size() << endl;

    for (unsigned int i = 0; i < stateList.size(); i++) {
	if (DEBUGREADXMLINPUT) {
	    debugfile << "State " << i << ": " << endl;
	    stateList[i].write(debugfile);
	    debugfile << endl;
	}
	// check if key already exits
	if (mymap.find(stateList[i].getVNamePrev()) == mymap.end()) {
	    mymap[stateList[i].getVNamePrev()] = &stateList[i];
	} else {
	    cerr << "ERROR\n  There seems to be repeated variable names for: "
		<< stateList[i].getVNamePrev() << endl;
	    cerr << "  Check pomdpX file for repeated variable names." << endl;
	    exit(XML_INPUT_ERROR);
	}
	if (mymap.find(stateList[i].getVNameCurr()) == mymap.end()) {
	    mymap[stateList[i].getVNameCurr()] = &stateList[i];
	} else {
	    cerr << "ERROR\n  There seems to be repeated variable names for: "
		<< stateList[i].getVNameCurr() << endl;
	    cerr << "  Check pomdpX file for repeated variable names." << endl;
	    exit(XML_INPUT_ERROR);
	}
    }

    if (DEBUGREADXMLINPUT)
	debugfile << "Observation List Size: " << observationList.size()
	    << endl;
    for (unsigned int i = 0; i < observationList.size(); i++) {
	if (DEBUGREADXMLINPUT) {
	    debugfile << "Observation " << i << ": " << endl;
	    observationList[i].write(debugfile);
	    debugfile << endl;
	}
	if (mymap.find(observationList[i].getVName()) == mymap.end()) {
	    mymap[observationList[i].getVName()] = &observationList[i];
	} else {
	    cerr << "ERROR\n  There seems to be repeated variable names for: "
		<< observationList[i].getVName() << endl;
	    cerr << "  Check pomdpX file for repeated variable names." << endl;

	    exit(XML_INPUT_ERROR);
	}
    }

    if (DEBUGREADXMLINPUT)
	debugfile << "Action List Size: " << actionList.size() << endl;
    for (unsigned int i = 0; i < actionList.size(); i++) {
	if (DEBUGREADXMLINPUT) {
	    debugfile << "Action " << i << ": " << endl;
	    actionList[i].write(debugfile);
	    debugfile << endl;
	}
	if (mymap.find(actionList[i].getVName()) == mymap.end()) {
	    mymap[actionList[i].getVName()] = &actionList[i];
	} else {	       
	    cerr << "ERROR\n  There seems to be repeated variable names for: "
		<< actionList[i].getVName() << endl;
	    cerr << "  Check pomdpX file for repeated variable names." << endl;
	    exit(XML_INPUT_ERROR);
	}
    }

    if (DEBUGREADXMLINPUT)
	debugfile << "Reward List Size: " << rewardList.size() << endl;

    for (unsigned int i = 0; i < rewardList.size(); i++) {
	if (DEBUGREADXMLINPUT) {
	    debugfile << "Reward " << i << ": " << endl;
	    rewardList[i].write(debugfile);
	    debugfile << endl;
	}
	if (mymap.find(rewardList[i].getVName()) == mymap.end()) {
	    mymap[rewardList[i].getVName()] = &rewardList[i];
	} else {
	    cerr << "ERROR\n  There seems to be repeated variable names for: "
		<< rewardList[i].getVName() << endl;
	    cerr << "  Check pomdpX file for repeated variable names." << endl;
	    exit(XML_INPUT_ERROR);
	}
    }


    if (DEBUGREADXMLINPUT)
	debugfile << "Terminal State Reward List Size: " << terminalStateRewardList.size() << endl;

    for (unsigned int i = 0; i < terminalStateRewardList.size(); i++) {
	if (DEBUGREADXMLINPUT) {
	    debugfile << "Terminal State Reward " << i << ": " << endl;
	    terminalStateRewardList[i].write(debugfile);
	    debugfile << endl;
	}
	if (mymap.find(terminalStateRewardList[i].getVName()) == mymap.end()) {
	    mymap[terminalStateRewardList[i].getVName()] = &terminalStateRewardList[i];
	} else {
	    cerr << "ERROR\n  There seems to be repeated variable names for: "
		<< terminalStateRewardList[i].getVName() << endl;
	    cerr << "  Check pomdpX file for repeated variable names." << endl;
	    exit(XML_INPUT_ERROR);
	}
    }



    // end of printing the variables information
    ///////////////////////////////////////////////


    TiXmlElement *pInitialStateBelief = hDoc.FirstChild("pomdpx").FirstChild(
	    "InitialStateBelief").FirstChild("CondProb").ToElement();
    while (pInitialStateBelief != 0) {

	Function nBeliefFunction = createFunction(pInitialStateBelief,BELIEFFUNCTION);

	beliefFunctionList.push_back(nBeliefFunction);

	string info;
	if (checkFunctionProbabilities(&nBeliefFunction, pInitialStateBelief, "Belief Function"))
	{
	    exit(XML_INPUT_ERROR);
	}

	pInitialStateBelief = pInitialStateBelief->NextSiblingElement();
    }//end of while


    if (beliefFunctionList.size() != stateList.size()) {
	cerr << "ERROR\n  The number of states do not correspond to the number of belief functions" << endl;
	exit(XML_INPUT_ERROR);
    }


    // the number of states in the state list should correspond to the number of transition
    // get state functions
    if (DEBUGREADXMLINPUT)
	debugfile << "\nState Functions: " << endl;


    TiXmlElement *pStructFunction = hDoc.FirstChild("pomdpx").FirstChild("StateTransitionFunction").ToElement();
    if(pStructFunction ==NULL){
	cerr << "ERROR\n  Cannot find StateTransitionFunction tag"<< endl;
	exit(XML_INPUT_ERROR);
    }
    TiXmlElement *pSF = pStructFunction->FirstChildElement();
    //TiXmlElement* pSF = pStructFunction->FirstChild()->ToElement();
    while (pSF != 0) {

	Function nStateFunction = createFunction(pSF,STATEFUNCTION);

	// checking that the probabilities sum up to 1
	string info;
	if (!(nStateFunction.checkNoMissingEntries(mymap, info))) {
	    printXMLErrorHeader(pSF);
	    cerr << "In State Function " << nStateFunction.getVNameCurr() << " : there are missing transitions" << endl;
	    cerr << info << endl;
	    exit(XML_INPUT_ERROR);
	}
	nStateFunction.sparseT->convertForUse();

	checkFunctionProbabilities(&nStateFunction, pSF, "State Function");

	// check that the parents follow our DBN model:
	if (!(validateModel(nStateFunction, info))) {
	    printXMLErrorHeader(pSF);
	    cerr << info << "  The input file does not match our DBN model" << endl;
	    exit(XML_INPUT_ERROR);
	}

	stateFunctionList.push_back(nStateFunction);
	if (DEBUGREADXMLINPUT)
	    nStateFunction.write(debugfile);

	pSF = pSF->NextSiblingElement();


    }//end of for-loop for state functions

    if (stateFunctionList.size() != stateList.size()) {
	printXMLErrorHeader(pStructFunction);
	cerr << "The number of states do not correspond to the number of state transition functions" << endl;
	exit(XML_INPUT_ERROR);
    }

    if (DEBUGREADXMLINPUT)
	debugfile << "\nObservation Functions: " << endl;

    if (observationList.size() > 0) {
	TiXmlElement *pObsFunction = hDoc.FirstChild("pomdpx").FirstChild("ObsFunction").ToElement();
	if(pObsFunction ==NULL){
	    cerr << "ERROR\n  Cannot find ObsFunction tag"<< endl;
	    exit(XML_INPUT_ERROR);
	}
	TiXmlElement *pOF = pObsFunction->FirstChild()->ToElement();

	while (pOF != 0) {

	    Function nFunction = createFunction(pOF, OBSERVFUNCTION);

	    // checking that the probabilities sum up to 1

	    string info;
	    if (!(nFunction.checkNoMissingEntries(mymap, info))) {
		printXMLErrorHeader(pOF);
		cerr << "In Observation Function " << nFunction.getVNameCurr() << " : there are missing transitions" << endl;
		cerr << info << endl;
		exit(XML_INPUT_ERROR);
	    }

	    nFunction.sparseT->convertForUse();
	    checkFunctionProbabilities(&nFunction, pOF, "Observation Function");

	    observFunctionList.push_back(nFunction);
	    if (DEBUGREADXMLINPUT)
		nFunction.write(debugfile);

	    pOF = pOF->NextSiblingElement();

	}//end of for-loop for observation functions

	if (observFunctionList.size() != observationList.size()) {
	    printXMLErrorHeader(pObsFunction);
	    cerr << "The number of observation variables do not correspond to the number of observations functions" << endl;
	    exit(XML_INPUT_ERROR);
	}

    }

    TiXmlElement *pRewardFunction = hDoc.FirstChild("pomdpx").FirstChild("RewardFunction").ToElement();
    if(pRewardFunction ==NULL){
	cerr << "ERROR\n  Cannot find RewardFunction tag"<< endl;
	exit(XML_INPUT_ERROR);
    }

    TiXmlElement *pRF = pRewardFunction->FirstChild()->ToElement();
    //printing reward functions
    if (DEBUGREADXMLINPUT)
	debugfile << "\nReward Functions: " << endl;

    while (pRF != 0) {

	Function rewardFunction = createFunction(pRF, REWARDFUNCTION);

	//rewardFunction.sparseT->sortEntries();
	//rewardFunction.sparseT->removeRedundant();
	//rewardFunction.sparseT->removeZeroEntries();

	//rewardFunction.addZeroEntries(mymap);
	//rewardFunction.sparseT->sortEntries();

	rewardFunction.sparseT->convertForUse();
	rewardFunctionList.push_back(rewardFunction);
	if (DEBUGREADXMLINPUT)
	    rewardFunction.write(debugfile);

	pRF = pRF->NextSiblingElement();

    }//end of for-loop for observation functions

    if (rewardFunctionList.size() != rewardList.size()) {
	printXMLErrorHeader(pRewardFunction);
	cerr << "The number of reward variables do not correspond to the number of reward functions" << endl;
	exit(XML_INPUT_ERROR);
    }



    if (DEBUGREADXMLINPUT)
	debugfile << "\nTerminal State Reward Functions: " << endl;

    TiXmlElement *pTerminalStateRewardFunction;
    if (terminalStateRewardList.size() > 0) {
	pTerminalStateRewardFunction = hDoc.FirstChild("pomdpx").FirstChild("TerminalStateRewardFunction").ToElement();

	TiXmlElement *pTSRF = pTerminalStateRewardFunction->FirstChild()->ToElement();
	//printing reward functions
	if (DEBUGREADXMLINPUT)
	    debugfile << "\nTerminal State Reward Functions: " << endl;

	while (pTSRF != 0) {

	    Function terminalStateRewardFunction = createFunction(pTSRF, TERMINALFUNCTION);

	    terminalStateRewardFunction.sparseT->sortEntries();
	    terminalStateRewardFunction.sparseT->removeRedundant();

	    terminalStateRewardFunctionList.push_back(terminalStateRewardFunction);
	    if (DEBUGREADXMLINPUT)
		terminalStateRewardFunction.write(debugfile);

	    pTSRF = pTSRF->NextSiblingElement();

	}
    }

    if (terminalStateRewardFunctionList.size() != terminalStateRewardList.size()) {
	cerr << "The number of terminal state reward variables do not correspond to the number of terminal state reward functions" << endl;
	exit(XML_INPUT_ERROR);
    }




    // mapping
    for (unsigned int i = 0; i < stateFunctionList.size(); i++) {
	mapFunc[stateFunctionList[i].getVNameCurr()] = &stateFunctionList[i];
    }

    for (unsigned int i = 0; i < observFunctionList.size(); i++) {
	mapFunc[observFunctionList[i].getVNameCurr()] = &observFunctionList[i];
    }

    for (unsigned int i = 0; i < rewardFunctionList.size(); i++) {
	mapFunc[rewardFunctionList[i].getVNameCurr()] = &rewardFunctionList[i];
    }

    for (unsigned int i = 0; i < terminalStateRewardFunctionList.size(); i++) {
	mapFunc[terminalStateRewardFunctionList[i].getVNameCurr()] = &terminalStateRewardFunctionList[i];
    }

    debugfile.close();
    //////END OF READING AND STORING INPUT FILE//////////////////////
    ////////////////////////////////////////////////////////////////

    return checkProblemType();

}//end of FactoredPomdp::start()

bool FactoredPomdp::checkFunctionProbabilities(Function* f, TiXmlElement* xmlNode, string whichFunction){
    vector<vector<int> > commonIndices;
    vector<double> probs;
    if (f->sparseT->errorInProbabilities(commonIndices, probs)) 
    {
	//flush error output immediately, in case we encounter a file with large number of error
	printXMLWarningHeader(xmlNode);
	cerr << "In "<< whichFunction <<" Tables " << f->getVNameCurr() << endl;
	for(int k=0;k<commonIndices.size();k++){
	    vector<int> commonIndex= commonIndices[k];
	    double prob = probs[k];
	    cerr << "  In instance ";
	    for(int i=0;i<commonIndex.size();i++){
		if(f->getParents()[i]!="null"){
		    StateObsAct* soa = mymap[f->getParents()[i]];
		    cerr << soa->getValueEnum()[commonIndex[i]] << " ";
		}else{
		    cerr << "null" << endl;
		}
	    }
	    cerr << endl;
	    cerr << "  Probabilities sum up to " <<  prob <<". It should sum up to 1" << endl;
	}
	return true;
    }
    else
	return false;
}

const int FactoredPomdp::checkProblemType() {

    //Checking if problem has fully unobserved states
    int numObserved = 0;
    int numUnobserved = 0;
    for (unsigned int i = 0; i < stateList.size(); i++) {
	if (stateList[i].getObserved())
	    numObserved++;
	else
	    numUnobserved++;
    }

    if (numUnobserved > 0) {
	if (numObserved == 0)
	    return FULLY_UNOBSERVED;
	else {
	    for (unsigned int i=0; i < stateFunctionList.size(); i++) {

		for(unsigned int j=0; j < stateFunctionList[i].sparseT->cIheader.size(); j++) {
		    // if the state function contains state variables in current time slice as parents
		    if (!(checkActionNameExists(stateFunctionList[i].sparseT->cIheader[j]))) {
			if (isCurrentTimeSlice(stateFunctionList[i].sparseT->cIheader[j])) {
			    cout << "MIXED REPARM since state function list has parents in current time slice" << endl;
			    return MIXED_REPARAM;
			}
		    }
		}
	    }

	    //check if initialbelief has currenttimeslice
	    for (unsigned int i=0; i < beliefFunctionList.size(); i++) {
		for (unsigned int j=0; j < beliefFunctionList[i].getParents().size(); j++) {
		    if (beliefFunctionList[i].getParents()[j] != "null") {
			cout << "MIXED REPARAM because belief function has parents that are not null" << endl;
			return MIXED_REPARAM;
		    }
		}
	    }


	    return MIXED;
	}
    } else {
	if (observationList.size() > 0) {
	    cerr << "WARNING\n  This problem is an MDP (all state variables are observed) but also contains observation variables. The observation variables will be ignored. "<< endl;
	}
	return FULLY_OBSERVED;
    }

}//end of checkProblemType()

void FactoredPomdp::sortStateList() {
    vector<State> newStateList;

    //Insert observed states first
    for (unsigned int i = 0; i < stateList.size(); i++) {
	if (stateList[i].getObserved())
	    newStateList.push_back(stateList[i]);
    }
    //then insert unobserved states
    for (unsigned int i = 0; i < stateList.size(); i++) {
	if (!(stateList[i].getObserved()))
	    newStateList.push_back(stateList[i]);
    }

    stateList = newStateList;
}//end of sortStateList

bool FactoredPomdp::validateModel(Function sf, string& info) {
    stringstream ssinfo;

    if (isPreviousTimeSlice(sf.getVNameCurr())) {
	ssinfo << "For State Transition Function "<< sf.getVNameCurr()<<", <Var> </Var> should not contain variables from the previous time slice" << endl;
	info = ssinfo.str();
	return false;
    }

    const State& sfState = findState(sf.getVNameCurr());
    if (sfState.getObserved()) {
	vector<string> parents = sf.getParents();
	for (unsigned int i=0; i < parents.size(); i++) {
	    if (!(checkActionNameExists(parents[i])) && isCurrentTimeSlice(parents[i])) {
		ssinfo << "If variable " << sf.getVNameCurr() << " is observed, it cannot have any parents in the SAME time slice: " << parents[i] << endl;
		info = ssinfo.str();
		return false;
	    }
	}

    }else {

	vector<string> parents = sf.getParents();
	for (unsigned int i=0; i < parents.size(); i++) {
	    if (!(checkActionNameExists(parents[i])) && isCurrentTimeSlice(parents[i])) {
		const State& tempS = findState(parents[i]);
		if (!(tempS.getObserved())) {
		    ssinfo << "If variable " << sf.getVNameCurr() << " is not observed, it cannot have any UNOBSERVED parents in its same time slice: " << parents[i] << endl;
		    info = ssinfo.str();
		    return false;
		}
	    }//end of if
	}//end of for
    }//end of else

    return true;

}//end of validateModel

const bool FactoredPomdp::checkRewardFunctionHasOnlyPreviousTimeSliceAndAction() const{

    for (unsigned int i=0; i < rewardFunctionList.size(); i++) {
	vector<string> parents = rewardFunctionList[i].getParents();
	for (unsigned int j=0; j<parents.size(); j++) {
	    if ((checkStateNameExists(parents[j]))) {
		if (isCurrentTimeSlice(parents[j])) return false;
	    }
	    if ((checkObsNameExists(parents[j]))) return false;
	}
    }
    return true;
}

//current time slice states or observation in reward functions
const set<string> FactoredPomdp::getRewardFunctionCurrentTimeSliceVars(Function* rewardFunction) {
    set<string> variables;
    vector<string> obsVariable;
    vector<string> parents = rewardFunction->getParents();
    for (unsigned int j=0; j<parents.size(); j++) {
	if ((checkStateNameExists(parents[j]))) {
	    if (isCurrentTimeSlice(parents[j])){
		variables.insert(parents[j]);
	    }
	}
	if ((checkObsNameExists(parents[j]))){
	    obsVariable.push_back(parents[j]);
	}
    }
    //if observation uses current time slice variable, add those variable
    for(vector<string>::iterator it=obsVariable.begin();it!=obsVariable.end();it++){
	Function obsFunction = *mapFunc[*it]; 
	vector<string> parents = obsFunction.getParents();
	for (unsigned int j=0; j<parents.size(); j++) {
	    if ((checkStateNameExists(parents[j]))) {
		if (isCurrentTimeSlice(parents[j])){
		    variables.insert(parents[j]);
		}
	    }
	}
	variables.insert(*it);
    }
    return variables;
}

const bool FactoredPomdp::isPreviousTimeSlice(string name) const{
    assert(stateList.size() > 0);
    for (unsigned int i = 0 ; i < stateList.size(); i++) {
	if (stateList[i].getVNamePrev() == name)
	    return true;
    }
    for (unsigned int i = 0 ; i < stateList.size(); i++) {
	if (stateList[i].getVNameCurr() == name)
	    return false;
    }
    cerr << "Not a State variable" << endl;
    assert(false);
}

const bool FactoredPomdp::isCurrentTimeSlice(string name) const{
    assert(stateList.size() > 0);
    for (unsigned int i = 0 ; i < stateList.size(); i++) {
	if (stateList[i].getVNameCurr() == name)
	    return true;
    }
    for (unsigned int i = 0 ; i < stateList.size(); i++) {
	if (stateList[i].getVNamePrev() == name)
	    return false;
    }
    cerr << "Not a State variable" << endl;
    assert(false);
}


void FactoredPomdp::convertFast() 
{
    DEBUG_LOG (cout << "convert fast" << endl; );

    convertFastStateTrans();

    if (observationList.size() == 0)
    {
	convertFastNoObservationsVariables();
    }
    else
    {
	convertFastObsTrans();
    }
    convertFastRewardTrans();
    convertFastBelief();
    convertFastVariables();

    DEBUG_LOG (cout << "convert fast subfunctions done" << endl; );

}


void FactoredPomdp::mapFastStatesToValue() {

    ////// part 1 of mapping
    /////// mapping the position of each index to an integer, so that we can convert it to the oldstate
    /////// to do this, i map the string to an integer, ie. robot_0 to 2, rock_0 to 1, and then
    //////  map 1(robot_0) to 2, 2(rock_0) to 1
    //////  for instance, robot_0 = 1 and rock_0=0 will be map to oldstate 3 for rock_2_1

    int increment = 1;
    for (int i = (int) stateList.size() - 1; i >= 0; i--) {
	positionStringIndexMap[stateList[i].getVNamePrev()] = increment;
	positionStringIndexMap[stateList[i].getVNameCurr()] = increment;
	increment *= stateList[i].getValueEnum().size();
    }
    // note: the value of increment is now the size of the "merged" states
    numMergedStates = increment;
}

void FactoredPomdp::mapFastIndexesToValues(SharedPointer<SparseTable> st) {

    // part 2 of mapping
    //string action = actionList[0].getVName();

    //unsigned int actionIndex = 0;

    // map the entries in the new table to particular integers to help calculate new position in old states
    for (unsigned int j = 0; j < st->cIheader.size(); j++) {
	if (!(checkActionNameExists(st->cIheader[j])))
	    fastPositionCIIndexMap[j] = positionStringIndexMap[st->cIheader[j]];
	else
	    fastPositionCIIndexMap[j] = actionStringIndexMap[st->cIheader[j]];
    }

    for (unsigned int j = 0; j < st->uIheader.size(); j++)
	fastPositionUIIndexMap[j]
	    = positionStringIndexMap[st->uIheader[j]];

    //end of mapping
    ////////////////////////
}


SharedPointer<SparseTable> FactoredPomdp::reduceUnmatchedCIWithUI(SharedPointer<SparseTable>  st, ofstream& debugfile, bool printDebugFile) {

    if (printDebugFile) {
	debugfile << "before re-param" << endl;
	st->write(debugfile);
	debugfile << endl;
    }


    for (unsigned int uIIndex=0; uIIndex < st->uIheader.size(); uIIndex++) {
	for (unsigned int cIIndex=0; cIIndex < st->cIheader.size(); cIIndex++) {
	    if (st->uIheader[uIIndex] == st->cIheader[cIIndex]) {
		st=st->removeUnmatchedCI(cIIndex, uIIndex);
	    }//end of if
	}//end of for
    }//end of for

    if (printDebugFile) {
	debugfile << "after re-param" << endl;
	st->write(debugfile);
	debugfile << endl;
    }

    return st;


}//end of reduceUnmatchedCIWithUI

void FactoredPomdp::resortFastStateTables(ofstream& debugfile, bool printDebugFile) {

    if (printDebugFile) {
	debugfile << "finalStateTable before fast re-sorting" << endl;
	finalStateTable->write(debugfile);
	debugfile << endl;
    }

    //re-sort the finalStateTable
    unsigned int pos;
    for (unsigned int i=0; i < actionList.size(); i++) {
	pos = finalStateTable->findPosition(actionList[i].getVName());
	finalStateTable->swapCIHeaders(i,pos);
	finalStateTable->swapSparseColumns(i,pos);
    }

    for (unsigned int i=0; i < stateList.size(); i++) {
	pos = finalStateTable->findPosition(stateList[i].getVNamePrev());
	finalStateTable->swapCIHeaders(i+actionList.size(),pos);
	finalStateTable->swapSparseColumns(i+actionList.size(),pos);
    }

    finalStateTable->sortEntries();

    if (printDebugFile) {
	debugfile << "finalStateTable after fast re-sorting" << endl;
	finalStateTable->write(debugfile);
	debugfile << endl;
    }
    //end of re-sorting

}//end of resortFastStateTables

void FactoredPomdp::convertFastStateTrans() {

    // also check for terminal states
    // using old merging table method in order to handle reparam and new method does not offer much advantage to unfactored
    ofstream debugfile;
    if (DEBUGFASTCONVERSIONSTATE) {
	debugfile.open("debug_FactoredPomdp_convertFastStateTrans.txt");
    }

    finalStateTable = mergeTables(&stateFunctionList, STATEFUNCTION, debugfile, /*DEBUGFASTCONVERSIONSTATE*/false);
    finalStateTable = reduceUnmatchedCIWithUI(finalStateTable, debugfile, /*DEBUGFACTOREDCONVERSIONSTATE*/false);
    resortFastStateTables(debugfile, /*DEBUGFACTOREDCONVERSIONSTATE*/false);
    defineCanonicalNames();

    mapActionsToValue();
    mapFastStatesToValue();
    mapFastIndexesToValues(finalStateTable);

    /////////////////////////////////
    // conversion to the old matrices
    /////////////////////////////////

    // initialising all the preSparseMatrix
    vector<PreSparseMatrix> cOstTrPre; // compressed old state transition transposed, (pre-conversion to c matrix)
    for (unsigned int i = 0; i < numActions; i++) {
	PreSparseMatrix cm(numMergedStates, numMergedStates);
	cOstTrPre.push_back(cm);
    }

    vector<PreSparseMatrix> cOstPre; // compressed old state transition, (pre-conversion to c matrix)
    for (unsigned int i = 0; i < numActions; i++) {
	PreSparseMatrix cm(numMergedStates, numMergedStates);
	cOstPre.push_back(cm);
    }

    int action;
    int startState;
    int endState;
    double prob;

    // these are for checking terminal states
    unsigned int *terminalStates = new unsigned int[numMergedStates];

    for (int i = 0; i < numMergedStates; i++)
	terminalStates[i] = 0;
    // end

    SparseEntry se;
    while(finalStateTable->getNext(se)){
	vector<int> commonIndex = finalStateTable->getIterPosition();
	action = startState = endState = 0;
	prob = 1;

	for (unsigned int j = 0; j < commonIndex.size(); j++) {
	    if (!(checkActionNameExists(finalStateTable->cIheader[j])))
		startState += commonIndex[j] * fastPositionCIIndexMap[j];
	    else
		action += commonIndex[j] * fastPositionCIIndexMap[j];
	}
	for (unsigned int j = 0; j < se.uniqueIndex.size(); j++) {
	    endState += se.uniqueIndex[j].index * fastPositionUIIndexMap[j];
	    prob *= se.uniqueIndex[j].value;
	}
	cOstTrPre[action].addEntries(endState, startState, prob);

	// for checking terminal states
	if ((endState == startState) && (fabs(prob - 1) < 0.000001))
	    terminalStates[endState]++;
	//end for checking terminal states

	cOstPre[action].addEntries(startState, endState,prob);
    }


    vector<SharedPointer<SparseMatrix> > cOst;
    for (unsigned int i = 0; i < cOstPre.size(); i++) 
    {
	cOst.push_back(cOstPre[i].convertSparseMatrix());
	if (DEBUGFASTCONVERSIONSTATE) {
	    debugfile << "compressed old state transition matrix" << i << endl;
	    cOst[i]->write(debugfile);
	    debugfile << endl;
	}
    }

    vector<SharedPointer<SparseMatrix> > cOstTr;
    for (unsigned int i = 0; i < cOstTrPre.size(); i++) {
	cOstTr.push_back(cOstTrPre[i].convertSparseMatrix());
	if (DEBUGFASTCONVERSIONSTATE) {
	    debugfile << "compressed old state transition transposed matrix"
		<< i << endl;
	    cOstTr[i]->write(debugfile);
	    debugfile << endl;
	}
    }

    layer.pomdpT = cOst;
    layer.pomdpTtr = cOstTr;

    // for checking terminal states
    vector<int> termStates;
    for (int i = 0; i < numMergedStates; i++) {
	if (terminalStates[i] == numActions)
	    termStates.push_back(1);
	else
	    termStates.push_back(0);
    }

    layer.pomdpIsPOMDPTerminalState = termStates;
    if (DEBUGFASTCONVERSIONSTATE) {
	debugfile << "terminal states" << endl;
	for (unsigned int i = 0; i < termStates.size(); i++) {
	    debugfile << termStates[i] << " ";
	}
    }

    debugfile.close();
    delete [] terminalStates;

}//end of void FactoredPomdp::convertFastStateTrans()

//////////////////////////////////////////////////////////////
// for conversion of R(x,y,x',y',a,o) to R(x,y,a)

SharedPointer<SparseTable> FactoredPomdp::preprocessRewardTable() 
{
    ofstream debugfile;
    if (DEBUGFACTOREDCONVERSIONREWARD) {
	debugfile.open("debug_FactoredPomdp_preprocessRewardTable.txt");
    }

    vector<SharedPointer<SparseTable> > rewardTables;

    //preprocess each reward table individually...
    for (unsigned int i=0; i < rewardFunctionList.size(); i++) {
	debugfile << "reward function " << i <<endl;

	//determine the functions required for conversion
	//i.e functions of current time slice variable used
	vector<Function> functionsDepend;
	set<string> curTimeVars = getRewardFunctionCurrentTimeSliceVars(&rewardFunctionList[i] );
	for(set<string>::iterator it=curTimeVars.begin();it!=curTimeVars.end();it++)
	{
	    functionsDepend.push_back(*mapFunc[*it]);
	}
	functionsDepend.push_back(rewardFunctionList[i]);

	//merge reward table with the functions it depends on
	SharedPointer<SparseTable> rewardTable = mergeTables(&functionsDepend, BELIEFFUNCTION, debugfile, DEBUGFACTOREDCONVERSIONREWARD); 
	rewardTable->write(debugfile);

	debugfile << "after reducing common indexes that are not matched with unique indexes" << endl;
	rewardTable = reduceUnmatchedCIWithUI(rewardTable, debugfile, DEBUGFACTOREDCONVERSIONREWARD);
	rewardTable->write(debugfile);

	debugfile << "after removing redundant unique indexes fron reward table" << endl;
	rewardTable = removeRedundantUIsFromReward(rewardTable);
	rewardTable->write(debugfile);

	rewardTables.push_back(rewardTable);
    }
    //merge the processed reward tables together
    SharedPointer<SparseTable> result = mergeSparseTables(rewardTables, REWARDFUNCTION, debugfile, DEBUGFACTOREDCONVERSIONREWARD);
    result->write(debugfile);
    return result;
}


void FactoredPomdp::convertFastObsTrans() {

    ofstream debugfile;
    if (DEBUGFASTCONVERSIONOBS) {
	debugfile.open("debug_FactoredPomdp_convertFastObsTrans.txt");
    }
    mapObservationsToValue();

    // Conversion to the old matrices
    vector<PreSparseMatrix> cOobsTrPre; // compressed old observation transition transposed, (pre-conversion to c matrix)
    for (unsigned int i = 0; i < numActions; i++) {
	PreSparseMatrix cm(numObservations, numMergedStates);
	cOobsTrPre.push_back(cm);
    }

    vector<PreSparseMatrix> cOobsPre; // compressed old observation transition, (pre-conversion to c matrix)
    for (unsigned int i = 0; i < numActions; i++) {
	PreSparseMatrix cm(numMergedStates, numObservations);
	cOobsPre.push_back(cm);
    }

    int action=0;
    int endState=0;
    map<string, int> variableValues = getStartActionSVarValues();
    do{
	vector<IndexProbTuple> observations;
	//merging the observations here
	for(vector<Function>::iterator obsFunc=observFunctionList.begin();obsFunc!=observFunctionList.end();obsFunc++){
	    //look up in each observation table, and merge the lookup results into index and probability

	    SharedPointer<SparseTable> obsTable = obsFunc->sparseT;
	    //assemble commom index for this table using current variable values
	    vector<int> commonIndex;
	    for(vector<string>::iterator cI=obsTable->cIheader.begin();cI!=obsTable->cIheader.end();cI++){
		commonIndex.push_back(variableValues[*cI]);
		//commonIndex.push_back(VNameValuesMap[*cI]);
	    }
	    vector<SparseEntry> entries = obsTable->getSparseEntries(commonIndex);

	    //create a new struct for first entry
	    if(observations.empty()){
		IndexProbTuple pt;
		pt.index = 0;
		pt.prob = 1;
		observations.push_back(pt);
	    }
	    vector<IndexProbTuple> temp = observations;
	    int lastSize = temp.size();	//last size of end states vector before repeating

	    //multiple the number of end state by number of entries in this row 
	    //repeat the possible end states if there are more than 1 entries for this row in this table
	    for(int i=1;i<entries.size();i++){
		observations.insert(observations.end(), temp.begin(), temp.end());
	    }

	    //merging each entry in this table into existing end states
	    for(int i=0;i<entries.size();i++){
		SparseEntry se = entries[i];
		double probMul=1;		//probability multiplier for this entry
		int obsInc=0;		//obseravtion number increment for this entry

		//merge the end state X and Y
		for (unsigned int j = 0; j < se.uniqueIndex.size(); j++) {
		    StateObsAct* soa = mymap[obsTable->uIheader[j]];
		    ObsAct* s = static_cast<ObsAct*> (soa);
		    obsInc += se.uniqueIndex[j].index * observationStringIndexMap[obsTable->uIheader[j]];
		    probMul *= se.uniqueIndex[j].value;
		}//end of for

		//merging increment and multiple from this entry into exisitng end states
		for(int k=i*lastSize;k<(i+1)*lastSize;k++){
		    observations[k].index += obsInc;
		    observations[k].prob *= probMul;
		}
	    }
	}
	for(vector<IndexProbTuple>::iterator pt=observations.begin();pt!=observations.end();pt++){
	    cOobsTrPre[action].addEntries(pt->index, endState, pt->prob);
	    cOobsPre[action].addEntries(endState, pt->index, pt->prob);
	}
    }while(getNextActionSVarValues(variableValues, action, endState));


    vector<SharedPointer<SparseMatrix> > cOobs;
    for (unsigned int i = 0; i < cOobsPre.size(); i++) {
	cOobs.push_back(cOobsPre[i].convertSparseMatrix());
	if (DEBUGFASTCONVERSIONOBS) {
	    debugfile << "compressed old observation transition matrix" << i
		<< endl;
	    cOobs[i]->write(debugfile);
	    debugfile << endl;
	}
    }

    vector<SharedPointer<SparseMatrix> > cOobsTr;
    for (unsigned int i = 0; i < cOobsTrPre.size(); i++) {
	cOobsTr.push_back(cOobsTrPre[i].convertSparseMatrix());
	if (DEBUGFASTCONVERSIONOBS) {
	    debugfile
		<< "compressed old observation transition transposed matrix"
		<< i << endl;
	    cOobsTr[i]->write(debugfile);
	    debugfile << endl;
	}
    }

    layer.pomdpO = cOobs;
    layer.pomdpOtr = cOobsTr;
    debugfile.close();
}//end of convertFastObsTrans()


void FactoredPomdp::convertFastNoObservationsVariables() {

    numObservations = 1;

    ofstream debugfile;
    if (DEBUGFASTCONVERSIONOBS) {
	debugfile.open("debug_FactoredPomdp_convertFastObsTrans.txt");
    }
    debugfile << "convertFastNoObservationsVariables()" << endl;

    // Conversion to the old matrices
    vector<PreSparseMatrix> cOobsTrPre; // compressed old observation transition transposed, (pre-conversion to c matrix)
    for (unsigned int i = 0; i < numActions; i++) {
	PreSparseMatrix cm(1, numMergedStates);
	cOobsTrPre.push_back(cm);
    }

    vector<PreSparseMatrix> cOobsPre; // compressed old observation transition, (pre-conversion to c matrix)

    for (unsigned int i = 0; i < numActions; i++) {
	PreSparseMatrix cm(numMergedStates, 1);
	cOobsPre.push_back(cm);
    }


    for (unsigned int i = 0 ; i < numActions; i++) {
	for (unsigned int j = 0 ; j < numMergedStates; j++) {
	    cOobsTrPre[i].addEntries(0, j, 1.0);
	    cOobsPre[i].addEntries(j, 0, 1.0);
	}
    }

    vector<SharedPointer<SparseMatrix> > cOobs;
    for (unsigned int i = 0; i < cOobsPre.size(); i++) {
	cOobs.push_back(cOobsPre[i].convertSparseMatrix());
	if (DEBUGFASTCONVERSIONOBS) {
	    debugfile << "compressed old observation transition matrix" << i
		<< endl;
	    cOobs[i]->write(debugfile);
	    debugfile << endl;
	}
    }

    vector<SharedPointer<SparseMatrix> > cOobsTr;
    for (unsigned int i = 0; i < cOobsTrPre.size(); i++) {
	cOobsTr.push_back(cOobsTrPre[i].convertSparseMatrix());
	if (DEBUGFASTCONVERSIONOBS) {
	    debugfile
		<< "compressed old observation transition transposed matrix"
		<< i << endl;
	    cOobsTr[i]->write(debugfile);
	    debugfile << endl;
	}
    }

    layer.pomdpO = cOobs;
    layer.pomdpOtr = cOobsTr;
    debugfile.close();
}//end of convertFastNoObsVariables()


void FactoredPomdp::convertFastRewardTrans() {

    ofstream debugfile;
    if (DEBUGFASTCONVERSIONREWARD) {
	debugfile.open("debug_FactoredPomdp_convertFastRewardTrans.txt");
    }

    if (!checkRewardFunctionHasOnlyPreviousTimeSliceAndAction()) {
	preprocessRewardFunction();
    }

    // Conversion to the old matrices
    PreSparseMatrix cOrewardPre(numMergedStates, numActions);

    int action = 0;
    int startState = 0;
    double reward = 0;

    map<string, int> variableValues = getStartActionSVarValues();
    do{  
	//vector storing reward from individual tables to be merged
	double reward = 0;
	for(vector<Function>::iterator rewardFunc=rewardFunctionList.begin();rewardFunc!=rewardFunctionList.end();rewardFunc++){
	    //look up in reward table, and merge the lookup results into reward 

	    SharedPointer<SparseTable> rewardTable = rewardFunc->sparseT;
	    //assemble commom index for this table using current variable values
	    vector<int> commonIndex;
	    for(vector<string>::iterator cI=rewardTable->cIheader.begin();cI!=rewardTable->cIheader.end();cI++){
		commonIndex.push_back(variableValues[*cI]);
	    }
	    vector<SparseEntry>& entries = rewardTable->getSparseEntries(commonIndex);
	    //merging each entry in this table into existing reward
	    for(int i=0;i<entries.size();i++){
		SparseEntry se = entries[i];

		//merge the end state X and Y
		for (unsigned int j = 0; j < se.uniqueIndex.size(); j++) {
		    reward += se.uniqueIndex[j].value;
		}//end of for
	    }
	}
	if (reward != 0) {
	    cOrewardPre.addEntries(startState, action, reward);
	}
    }while(getNextActionSVarValues(variableValues,action,startState));

    layer.pomdpR = cOrewardPre.convertSparseMatrix();
    if (DEBUGFASTCONVERSIONREWARD) {
	debugfile << "compressed old reward matrix" << endl;
	layer.pomdpR->write(debugfile);
	debugfile << endl;
    }

    debugfile.close();
}//end of convertFastRewardTrans()

void FactoredPomdp::convertFastBelief() 
{
    ofstream debugfile;  
    if (DEBUGFASTCONVERSIONBELIEF)
	debugfile.open("debug_FactoredPomdp_convertFastBelief.txt");
    if (DEBUGFASTCONVERSIONBELIEF)
	debugfile << "Converting Fast Belief State" << endl;

    // combine all the different state tables to form a big one

    preprocessBeliefTables(debugfile, DEBUGFASTCONVERSIONBELIEF);
    mergeBeliefTables(debugfile, DEBUGFASTCONVERSIONBELIEF);

    finalBeliefTable->sortEntries();

    mapFastIndexesToValues(finalBeliefTable);

    SparseVector cv(numMergedStates);
    vector<double> vec_cv(numMergedStates,0.0);

    int startState, endState;
    double prob;

    SparseEntry se;
    while(finalBeliefTable->getNext(se))
    {
	startState = endState = 0;
	prob = 1.0;
	vector<int> commonIndex = finalBeliefTable->getIterPosition();
	//startState from 0
	for (unsigned int j = 0; j < commonIndex.size(); j++) 
	{
	    startState += commonIndex[j] * fastPositionCIIndexMap[j];
	}

	for (unsigned int j = 0; j < se.uniqueIndex.size(); j++) 
	{
	    endState += se.uniqueIndex[j].index * fastPositionUIIndexMap[j];
	    prob *= se.uniqueIndex[j].value;
	}//end of for

	if (startState != endState) 
	{
	    cerr << "for initial belief state, the entries should have common indexes correspoding to unique indexes" << endl;
	    assert(false);
	}

	vec_cv[startState] = prob;

    }//end of for
    finalBeliefTable = NULL;
    for (unsigned int i=0 ; i < vec_cv.size(); i++) {
	if (fabs(vec_cv[i]) > 0.000001) {
	    cv.push_back(i, vec_cv[i]);
	}
    }
    layer.pomdpInitialBelief = cv;

    if (DEBUGFASTCONVERSIONBELIEF)
	layer.pomdpInitialBelief.write(debugfile);

    debugfile << endl;
    debugfile.close();

}//end of convertfastbelief

void FactoredPomdp::convertFastVariables() {

    ofstream debugfile;
    if (DEBUGFASTCONVERSIONOTHERS)
	debugfile.open("debug_FactoredPomdp_convertFastOthers.txt");

    layer.pomdpDiscount = discount;
    if (DEBUGFASTCONVERSIONOTHERS)
	debugfile << "\ndiscount: " << layer.pomdpDiscount << endl;

    layer.pomdpNumActions = numActions;
    if (DEBUGFASTCONVERSIONOTHERS)
	debugfile << "num actions: " << layer.pomdpNumActions << endl;
    layer.pomdpNumObservations = numObservations;
    if (DEBUGFASTCONVERSIONOTHERS)
	debugfile << "num observations: " << layer.pomdpNumObservations << endl;
    layer.pomdpNumStates = layer.pomdpInitialBelief.size();
    //layer.numStateDimensions = fastPomdp.initialBelief.size_;
    if (DEBUGFASTCONVERSIONOTHERS)
	debugfile << "num states: " << layer.pomdpNumStates << endl;
    if (DEBUGFASTCONVERSIONOTHERS)
	debugfile << "num states dimensions: " << layer.pomdpNumStates
	    << endl;

    debugfile.close();
}//end of convert variables


//////////////////////////////////////////////
//////////////////// Don't change the order of calling
void FactoredPomdp::convertFactored() {

    DEBUG_LOG( cout << "convert factored" << endl; );
    convertFactoredStateTrans();
    if (observationList.size() == 0)
	convertFactoredNoObservationsVariables();
    else
	convertFactoredObsTrans();
    convertFactoredRewardTrans();

    if (terminalStateRewardList.size() >0)
	convertFactoredTerminalStateReward();

    convertFactoredBelief();

    convertFactoredVariables();

    DEBUG_LOG( cout << "convert factored subfunctions done" << endl; );

}

void FactoredPomdp::convertFactoredReparam() {


    cout << "convert factored reparam" << endl;
    convertFactoredStateReparamTrans();

    if (observationList.size() == 0)
	convertFactoredNoObservationsVariables();
    else
	convertFactoredObsTrans();

    convertFactoredRewardTrans();
    convertFactoredBeliefReparam();
    convertFactoredVariables();

    //cout << "exiting.. calling convert fast" << endl;
    //tobecommented
    //convertFast();
    //endooftobecommented
    //cout << "convert fast done" << endl;

}
SharedPointer<SparseTable>  FactoredPomdp::mergeSparseTables(vector<SharedPointer<SparseTable> > stList, int whichFunction, ofstream& debugfile, bool printDebugFile) {

    for (unsigned int i = 0; i < stList.size(); i++) {

	if (printDebugFile) {
	    debugfile << "function" << i << endl;
	    stList[i]->write(debugfile);
	    debugfile << endl;
	}
    }//end of for

    SharedPointer<SparseTable>  resultTable = stList[0];
    for (unsigned int i = 1; i < stList.size(); i++) 
    {
	resultTable = SparseTable::join(*resultTable,*stList[i], whichFunction);

	if (printDebugFile) 
	{
	    debugfile << "Intermediate table " << i << endl;
	    resultTable->write(debugfile);
	    debugfile << endl;
	}
    }//end of for
    return resultTable;

}//end of mergeTables

SharedPointer<SparseTable>  FactoredPomdp::mergeTables(vector<Function>* functionList, int whichFunction, ofstream& debugfile, bool printDebugFile) {

    for (unsigned int i = 0; i < functionList->size(); i++) {

	if (printDebugFile) {
	    debugfile << "function" << i << endl;
	    (*functionList)[i].sparseT->write(debugfile);
	    //debugfile << (*functionList)[i].sparseT->getInfo();
	    debugfile << endl;
	}
    }//end of for
    // TODO: check with shao wei, why only the first function get sorted
    (*functionList)[0].sparseT->sortEntries();

    //do deep cloning first to check result with testsuite
    //to be reverted to cheap cloning once testing is over
    SharedPointer<SparseTable>  resultTable =  (*functionList)[0].sparseT;
    //SharedPointer<SparseTable>  resultTable(new SparseTable(*(*functionList)[0].sparseT));
    resultTable->sortEntries();
    for (unsigned int i = 1; i < functionList->size(); i++) 
    {
	resultTable = SparseTable::join(*resultTable,*(*functionList)[i].sparseT, whichFunction);

	if (printDebugFile) 
	{
	    debugfile << "Intermediate table " << i << endl;
	    resultTable->write(debugfile);
	    //debugfile << resultTable->getInfo();
	    debugfile << endl;
	}
    }//end of for
    return resultTable;

}//end of mergeTables

// needed for reparam problems
void FactoredPomdp::expandFactoredStateTable(SharedPointer<SparseTable> sf) {

    vector<string> cIheader = sf->cIheader;
    int oldNumCI =  sf->cIheader.size();
    vector<int> numCIValues = sf->numCIValues;
    for (unsigned int i = 0; i < stateList.size() ; i++) {

	if ((!(sf->containsCI(stateList[i].getVNameCurr()))) && (stateList[i].getObserved()))  {
	    cIheader.push_back(stateList[i].getVNameCurr());

	    StateObsAct* soa = mymap[stateList[i].getVNameCurr()];
	    State* s = static_cast<State*> (soa);
	    numCIValues.push_back(s->getValueEnum().size()); 

	}//end of if
    }//end of for

    SharedPointer<SparseTable> expandedTable (new SparseTable(cIheader, sf->uIheader, numCIValues, sf->numUIValues));
    vector<int> newCI = expandedTable->getIterBegin(); 
    do{
	//get the CI of old table to copy from
	vector<int> oldCI;
	for(int k=0;k<oldNumCI;k++){
	    oldCI.push_back(newCI[k]);
	}
	//copy and repeat old sparse entries in expanded table
	vector<SparseEntry>& curRow = sf->getSparseEntries(oldCI);
	for(int j=0;j<curRow.size();j++){
	    expandedTable->add(newCI, curRow[j]); 
	}
    }while(expandedTable->getNextCI(newCI));

    sf = expandedTable; 
}//end of expandFactoredSparseTable

void FactoredPomdp::resortFactoredStateTables(ofstream& debugfile, bool printDebugFile, const int MIXEDTYPE) {

    if (printDebugFile) {
	debugfile << "finalStateTable before factored re-sorting" << endl;
	finalStateTable->write(debugfile);
	debugfile << endl;
    }

    //re-sort the finalStateTable
    unsigned int pos;
    for (unsigned int i=0; i < actionList.size(); i++) {
	pos = finalStateTable->findPosition(actionList[i].getVName());
	finalStateTable->swapCIHeaders(i,pos);
	finalStateTable->swapSparseColumns(i,pos);
    }

    // sort all parents in previous time slice
    for (unsigned int i=0; i < stateList.size(); i++) {
	pos = finalStateTable->findPosition(stateList[i].getVNamePrev());
	finalStateTable->swapCIHeaders(i+actionList.size(),pos);
	finalStateTable->swapSparseColumns(i+actionList.size(),pos);
    }

    if (MIXEDTYPE == MIXED_REPARAM) {
	// followed by those in current time slice(only observed variables)
	for (unsigned int i=0; i < stateList.size(); i++) {
	    if (stateList[i].getObserved()) {
		pos = finalStateTable->findPosition(stateList[i].getVNameCurr());
		finalStateTable->swapCIHeaders(i+actionList.size()+stateList.size(),pos);
		finalStateTable->swapSparseColumns(i+actionList.size()+stateList.size(),pos);
	    }
	}
    }

    finalStateTable->sortEntries();

    if(printDebugFile) {
	debugfile << "finalStateTable after factored re-sorting" << endl;
	finalStateTable->write(debugfile);
	debugfile << endl;
    }
    //end of re-sorting

}//end of resortFactoredStateTables



const void FactoredPomdp::defineCanonicalNames() {

    // we assume that the final state table contains all the state variables in CI

    //this is not really necessary, but if the function is called by convertFactored followed by convertFast(shouldn't be this case but for convenience, i rather call both methods for debugging), we need to clear the vectors
    canonicalNamePrev.clear();
    canonicalNameCurr.clear();
    canonicalNameForTerminal.clear();


    // for random ordering
    for (unsigned int j = 0; j < finalStateTable->cIheader.size(); j++) {
	// note that all in current time slices are not added
	if (checkActionNameExists(finalStateTable->cIheader[j])) 
	    canonicalNamePrev.push_back(finalStateTable->cIheader[j]);
	else if (isPreviousTimeSlice(finalStateTable->cIheader[j]))
	    canonicalNamePrev.push_back(finalStateTable->cIheader[j]);
    }

    for (unsigned int j = 0; j < finalStateTable->cIheader.size(); j++) {
	if (checkActionNameExists(finalStateTable->cIheader[j])) {
	    canonicalNameCurr.push_back(finalStateTable->cIheader[j]);
	}else{
	    if (isPreviousTimeSlice(finalStateTable->cIheader[j])) {
		const State& s = findState(finalStateTable->cIheader[j]);
		canonicalNameCurr.push_back(s.getVNameCurr());
	    }
	}
    }

    for (unsigned int j = 0; j < finalStateTable->cIheader.size(); j++) {
	if (!(checkActionNameExists(finalStateTable->cIheader[j])))
	    if (isPreviousTimeSlice(finalStateTable->cIheader[j]))
		canonicalNameForTerminal.push_back(finalStateTable->cIheader[j]);
    }


}//end of defineCanonicalNames

void FactoredPomdp::mapActionsToValue() {

    int increment = 1;
    for (int i = (int) actionList.size() - 1; i >= 0; i--) {
	actionStringIndexMap[actionList[i].getVName()] = increment;
	increment *= actionList[i].getValueEnum().size();
    }
    numActions = increment;

}//end of mapActionsToValue()

void FactoredPomdp::mapObservationsToValue() {

    int increment = 1;
    for (int i = (int) observationList.size() - 1; i >= 0; i--) {
	observationStringIndexMap[observationList[i].getVName()] = increment;
	increment *= observationList[i].getValueEnum().size();
    }
    numObservations = increment;
}//end of mapObservationsToValue()


void FactoredPomdp::mapObservationsUIsToValue(SharedPointer<SparseTable> st) {

    observationUIIndexMap.clear();

    for (unsigned int j = 0; j < st->uIheader.size(); j++) {

	StateObsAct* soa = mymap[st->uIheader[j]];
	observationUIIndexMap[j] = observationStringIndexMap[st->uIheader[j]];

    }
    //end of mapping
}

void FactoredPomdp::mapFactoredStatesToValue() {

    ///////////// mapping
    /////// mapping the position of each index to an integer, so that we can convert it to the oldstate
    /////// we map separately the indexes of observed states and unobserved states

    int incrementX = 1;
    int incrementY = 1;

    for (int i = (int) stateList.size() - 1; i >= 0; i--) {
	const State& s = stateList[i];
	if (s.getObserved()) {// observed
	    positionXStringIndexMap[stateList[i].getVNamePrev()] = incrementX;
	    positionXStringIndexMap[stateList[i].getVNameCurr()] = incrementX;
	    incrementX *= stateList[i].getValueEnum().size();
	} else {
	    positionYStringIndexMap[stateList[i].getVNamePrev()] = incrementY;
	    positionYStringIndexMap[stateList[i].getVNameCurr()] = incrementY;
	    incrementY *= stateList[i].getValueEnum().size();
	}
    }
    // note: the value of increment is now the size of the "merged" states
    numMergedStatesX = incrementX;
    numMergedStatesY = incrementY;
}//end of mapFactoredStringsToValue

void FactoredPomdp::mapFactoredCIsToValue(SharedPointer<SparseTable> st) {

    factoredPositionCIIndexMap.clear();

    // map the entries in the new table to particular integers to help calculate new position in old states

    for (unsigned int j = 0; j < st->cIheader.size(); j++) {
	if (!(checkActionNameExists(st->cIheader[j]))) {
	    StateObsAct* soa = mymap[st->cIheader[j]];
	    State* s = static_cast<State*> (soa);
	    if (s->getObserved()) {
		factoredPositionCIIndexMap[j]
		    = positionXStringIndexMap[st->cIheader[j]];
	    } else {
		factoredPositionCIIndexMap[j]
		    = positionYStringIndexMap[st->cIheader[j]];
	    }
	}else{
	    factoredPositionCIIndexMap[j] = actionStringIndexMap[st->cIheader[j]];
	}
    }//end of for
}

void FactoredPomdp::mapFactoredStateUIsToValue(SharedPointer<SparseTable> st) {

    factoredPositionUIIndexMap.clear();

    for (unsigned int j = 0; j < st->uIheader.size(); j++) {

	StateObsAct* soa = mymap[st->uIheader[j]];
	State* s = static_cast<State*> (soa);
	if (s->getObserved())
	    factoredPositionUIIndexMap[j]
		= positionXStringIndexMap[st->uIheader[j]];
	else
	    factoredPositionUIIndexMap[j]
		= positionYStringIndexMap[st->uIheader[j]];

    }
    //end of mapping
}


void FactoredPomdp::mapFactoredBeliefIndexesToValue(SharedPointer<SparseTable> st) {

    factoredPositionCIIndexMap.clear();
    factoredPositionUIIndexMap.clear();

    // start from 0 
    for (unsigned int j = 0; j < st->cIheader.size(); j++) {

	StateObsAct* soa = mymap[st->cIheader[j]];
	State* s = static_cast<State*> (soa);
	if (s->getObserved()) {
	    factoredPositionCIIndexMap[j]
		= positionXStringIndexMap[st->cIheader[j]];
	} else {
	    factoredPositionCIIndexMap[j]
		= positionYStringIndexMap[st->cIheader[j]];
	}

    }//end of for

    for (unsigned int j = 0; j < st->uIheader.size(); j++) {

	StateObsAct* soa = mymap[st->uIheader[j]];
	State* s = static_cast<State*> (soa);
	if (s->getObserved())
	    factoredPositionUIIndexMap[j]
		= positionXStringIndexMap[st->uIheader[j]];
	else
	    factoredPositionUIIndexMap[j]
		= positionYStringIndexMap[st->uIheader[j]];
    }
    //end of mapping
}

void FactoredPomdp::convertFactoredStateReparamTrans() {

    ofstream debugfile;
    if (DEBUGFACTOREDCONVERSIONSTATE) {
	debugfile.open("debug_FactoredPomdp_convertFactoredState.txt");
	//	  debugfile << "Convert Factored State Reparam\n" << endl;
    }

    mapActionsToValue();
    mapFactoredStatesToValue();

    // Conversion to the old matrices
    // TXtr[a][x](x',y),
    // compressed old state transition transposed, (pre-conversion to c matrix)
    vvPreSparseMatrix cOstXTrPre = createVvPreSparseMatrix(numActions, numMergedStatesX, numMergedStatesX, numMergedStatesY); 
    // TX[a][x](y,x')
    vvPreSparseMatrix cOstXPre = createVvPreSparseMatrix(numActions, numMergedStatesX, numMergedStatesY, numMergedStatesX); // compressed old state transition, (pre-conversion to c matrix)

    //TYtr[a][x][x'](y',y)
    vector<vvPreSparseMatrix> cOstYTrPre; // compressed old state transition transposed, (pre-conversion to c matrix)
    for (unsigned int i = 0; i < numActions; i++) {
	vvPreSparseMatrix tempX = createVvPreSparseMatrix(numMergedStatesX, numMergedStatesX, numMergedStatesY, numMergedStatesY);
	cOstYTrPre.push_back(tempX);
    }

    //TY[a][x][x'](y,y')
    vector<vvPreSparseMatrix > cOstYPre; // compressed old state transition, (pre-conversion to c matrix)
    for (unsigned int i = 0; i < numActions; i++) {
	vvPreSparseMatrix tempX = createVvPreSparseMatrix(numMergedStatesX, numMergedStatesX, numMergedStatesY, numMergedStatesY);
	cOstYPre.push_back(tempX);
    }//end of for

    int action;
    int startStateX; 
    int startStateY; 
    int startStateX_prime;

    // these are for checking terminal states
    unsigned int *terminalStatesX = new unsigned int[numMergedStatesX];
    for (int i = 0; i < numMergedStatesX; i++)
	terminalStatesX[i] = 0;

    unsigned int *terminalStatesY = new unsigned int[numMergedStatesY];
    for (int i = 0; i < numMergedStatesY; i++)
	terminalStatesY[i] = 0;
    // end
    map<string, int> variableValues = getStartActionXYVarValues();
    action = startStateX = startStateX_prime = startStateY = 0;
    do{  
	//merging the end state here
	vector<EndState> endStates; 
	for(vector<Function>::iterator stateFunc=stateFunctionList.begin();stateFunc!=stateFunctionList.end();stateFunc++){
	    //look up in each state table, and merge the lookup results into endStateX and endStateY

	    SharedPointer<SparseTable> stateTable = stateFunc->sparseT;
	    //assemble commom index for this table using current variable values
	    vector<int> commonIndex;
	    for(vector<string>::iterator cI=stateTable->cIheader.begin();cI!=stateTable->cIheader.end();cI++){
		commonIndex.push_back(variableValues[*cI]);
		//commonIndex.push_back(VNameValuesMap[*cI]);
	    }

	    vector<SparseEntry> entries = stateTable->getSparseEntries(commonIndex);

	    //create a new end state struct for first entry
	    if(endStates.empty()){
		EndState es;
		es.endStateX = 0;
		es.endStateY = 0;
		es.probX = 1;
		es.probY = 1;
		endStates.push_back(es);
	    }
	    vector<EndState> temp = endStates;
	    int lastSize = temp.size();	//last size of end states vector before repeating

	    //multiple the number of end state by number of entries in this row 
	    //repeat the possible end states if there are more than 1 entries for this row in this table
	    for(int i=1;i<entries.size();i++){
		endStates.insert(endStates.end(), temp.begin(), temp.end());
	    }

	    //merging each entry in this table into existing end states
	    for(int i=0;i<entries.size();i++){
		SparseEntry se = entries[i];
		double probXmul, probYmul;		//probability multiplier for this entry
		int endStateXinc, endStateYinc;		//state number increment for this entry
		probXmul= probYmul= 1;
		endStateXinc = endStateYinc = 0; 

		//merge the end state X and Y
		for (unsigned int j = 0; j < se.uniqueIndex.size(); j++) {

		    StateObsAct* soa = mymap[stateTable->uIheader[j]];
		    State* s = static_cast<State*> (soa);

		    if (s->getObserved()) {
			endStateXinc += se.uniqueIndex[j].index * positionXStringIndexMap[stateTable->uIheader[j]];
			probXmul *= se.uniqueIndex[j].value;
		    } else {
			endStateYinc += se.uniqueIndex[j].index * positionYStringIndexMap[stateTable->uIheader[j]];
			probYmul *= se.uniqueIndex[j].value;
		    }
		}//end of for


		//merging increment and multiple from this entry into exisitng end states
		for(int k=i*lastSize;k<(i+1)*lastSize;k++){
		    endStates[k].endStateX += endStateXinc;
		    endStates[k].endStateY += endStateYinc;
		    endStates[k].probX *= probXmul;
		    endStates[k].probY *= probYmul;
		}
	    }

	}

	for(vector<EndState>::iterator es=endStates.begin();es!=endStates.end();es++){
	    // TXtr[a][x](x',y),
	    cOstXTrPre[action][startStateX].addEntries(es->endStateX, startStateY, es->probX);
	    // TX[a][x](y,x')
	    cOstXPre[action][startStateX].addEntries(startStateY, es->endStateX, es->probX);
	    //TYtr[a][x][x'](y',y)
	    cOstYTrPre[action][startStateX][startStateX_prime].addEntries(es->endStateY, startStateY, es->probY);
	    //TY[a][x][x'](y,y')
	    cOstYPre[action][startStateX][startStateX_prime].addEntries(startStateY, es->endStateY, es->probY);

	    // for checking terminal states
	    if ((es->endStateX == startStateX) && (fabs(es->probX - 1) < 0.000001))
		terminalStatesX[es->endStateX]++;
	    if ((es->endStateY == startStateY) && (fabs(es->probY - 1) < 0.000001))
		terminalStatesY[es->endStateY]++;
	    //end for checking terminal states
	}
    }while(getNextActionXXpYVarValues(variableValues,action,startStateX,startStateX_prime, startStateY));

    //TX[a][x](y,x')
    vvSparseMatrix cOstX = helperPreSparseMatrixToSparseMatrix(cOstXPre);

    if (DEBUGFACTOREDCONVERSIONSTATE) {
	for (unsigned int i = 0; i < cOstX.size(); i++) {
	    for (unsigned int j = 0; j < cOstX[0].size(); j++) {
		debugfile << "cOstX ( TX[a][x](y,x') ), action: " << i
		    << " x: " << j << endl;
		cOstX[i][j]->write(debugfile);
		debugfile << endl;
	    }
	}
    }//end of if

    // TXtr[a][x](x',y)
    vvSparseMatrix cOstXTr = helperPreSparseMatrixToSparseMatrix(cOstXTrPre);

    if (DEBUGFACTOREDCONVERSIONSTATE) {
	for (unsigned int i = 0; i < cOstXTr.size(); i++) {
	    for (unsigned int j = 0; j < cOstXTr[0].size(); j++) {
		debugfile << "cOstXTr ( TXtr[a][x](x',y) ), action: " << i
		    << "x: " << j << endl;
		cOstXTr[i][j]->write(debugfile);
		debugfile << endl;
	    }
	}
    }//end of if

    //TY[a][x][x'](y,y')
    vector<vvSparseMatrix > cOstY = helperPreSparseMatrixToSparseMatrix(cOstYPre);

    if (DEBUGFACTOREDCONVERSIONSTATE) {
	for (unsigned int i = 0; i < cOstY.size(); i++) {
	    for (unsigned int j = 0; j < cOstY[0].size(); j++) {
		for (unsigned int k = 0; k < cOstY[0][0].size(); k++) {
		    debugfile << "cOstY ( TY[a][x][x'](y,y') ), action: " << i << "x: "
			<< j << "x': " << k << endl;
		    cOstY[i][j][k]->write(debugfile);
		    debugfile << endl;
		}
	    }
	}
    }//end of if


    //TYtr[a][x][x'](y',y)
    vector<vvSparseMatrix > cOstYTr = helperPreSparseMatrixToSparseMatrix(cOstYTrPre);

    if (DEBUGFACTOREDCONVERSIONSTATE) {
	for (unsigned int i = 0; i < cOstYTr.size(); i++) {
	    for (unsigned int j = 0; j < cOstYTr[0].size(); j++) {
		for (unsigned int k = 0 ; k < cOstYTr[0][0].size(); k++) {
		    debugfile << "cOstYTr ( TYtr[a][x][x'](y',y) ), action: " << i << "x: "
			<< j << "x': " << k << endl;
		    cOstYTr[i][j][k]->write(debugfile);
		    debugfile << endl;
		}
	    }
	}
    }


    // for checking terminal states
    vector<int> termStatesX;
    for (int i = 0; i < numMergedStatesX; i++) {
	// the number should correspond to all the number of entries
	if (terminalStatesX[i] == numActions * numMergedStatesY * numMergedStatesX)
	    termStatesX.push_back(1);
	else
	    termStatesX.push_back(0);
    }

    vector<int> termStatesY;
    for (int i = 0; i < numMergedStatesY; i++) {
	if (terminalStatesY[i] == numActions * numMergedStatesX * numMergedStatesX)
	    termStatesY.push_back(1);
	else
	    termStatesY.push_back(0);
    }

    if (DEBUGFACTOREDCONVERSIONSTATE) {
	debugfile << "terminal states X" << endl;
	for (unsigned int i = 0; i < termStatesX.size(); i++) {
	    debugfile << termStatesX[i] << " ";
	}

	debugfile << "\nterminal states Y" << endl;
	for (unsigned int i = 0; i < termStatesY.size(); i++) {
	    debugfile << termStatesY[i] << " ";
	}

    }

    layer.TX = cOstX;
    layer.TXtr = cOstXTr;
    layer.TY_reparam = cOstY;
    layer.TYtr_reparam = cOstYTr;

    layer.isPOMDPTerminalState.push_back(termStatesX);
    layer.isPOMDPTerminalState.push_back(termStatesY);

    debugfile.close();



}//end of void FactoredPomdp::convertFactoredStateReparamTrans()


//create a pre SparseMatrix of the size M[a][b](c,d)
vvPreSparseMatrix FactoredPomdp::createVvPreSparseMatrix(int a, int b, int c, int d){
    vvPreSparseMatrix M; // compressed old state transition transposed, (pre-conversion to c matrix)
    for (unsigned int i = 0; i <a; i++) {
	vector<PreSparseMatrix> temp;
	for (int j = 0; j < b; j++) {
	    PreSparseMatrix cm(c, d);
	    temp.push_back(cm);
	}
	M.push_back(temp);
    }
    return M;
}

void FactoredPomdp::printSparseMatrix(string title, vvSparseMatrix M, ofstream& debugfile){
    if (DEBUGFACTOREDCONVERSIONSTATE) {
	for (unsigned int i = 0; i < M.size(); i++) {
	    for (unsigned int j = 0; j < M[0].size(); j++) {
		debugfile << title << i
		    << "x: " << j << endl;
		M[i][j]->write(debugfile);
		debugfile << endl;
	    }
	}
    }//end of if
}


void FactoredPomdp::convertFactoredStateTrans() 
{
    ofstream debugfile;
    if (DEBUGFACTOREDCONVERSIONSTATE) {
	debugfile.open("debug_FactoredPomdp_convertFactoredState.txt");
	//	  debugfile << "Convert Factored State\n" << endl;
    }
    mapActionsToValue();
    mapFactoredStatesToValue();

    // Conversion to the old matrices
    // TXtr[a][x](x',y),
    // compressed old state transition transposed, (pre-conversion to c matrix)
    vvPreSparseMatrix cOstXTrPre = createVvPreSparseMatrix(numActions, numMergedStatesX, numMergedStatesX, numMergedStatesY); 
    // TX[a][x](y,x')
    vvPreSparseMatrix cOstXPre = createVvPreSparseMatrix(numActions, numMergedStatesX, numMergedStatesY, numMergedStatesX); 
    //TYtr[a][x](y',y)
    vvPreSparseMatrix cOstYTrPre = createVvPreSparseMatrix(numActions, numMergedStatesX, numMergedStatesY, numMergedStatesY); 
    //TY[a][x](y,y')
    vvPreSparseMatrix cOstYPre = createVvPreSparseMatrix(numActions, numMergedStatesX, numMergedStatesY, numMergedStatesY) ; 

    int action, startStateX, startStateY;

    // these are for checking terminal states
    unsigned int *terminalStatesX = new unsigned int[numMergedStatesX];
    for (int i = 0; i < numMergedStatesX; i++)
	terminalStatesX[i] = 0;

    unsigned int *terminalStatesY = new unsigned int[numMergedStatesY];
    for (int i = 0; i < numMergedStatesY; i++)
	terminalStatesY[i] = 0;
    // end
    //
    //enumerate thru all the variable values
    map<string, int> variableValues = getStartActionXYVarValues();
    action = startStateX = startStateY = 0;
    do{  
	//merging the end state here
	vector<EndState> endStates; 
	for(vector<Function>::iterator stateFunc=stateFunctionList.begin();stateFunc!=stateFunctionList.end();stateFunc++){
	    //look up in each state table, and merge the lookup results into endStateX and endStateY

	    SharedPointer<SparseTable> stateTable = stateFunc->sparseT;
	    //assemble commom index for this table using current variable values
	    vector<int> commonIndex;
	    for(vector<string>::iterator cI=stateTable->cIheader.begin();cI!=stateTable->cIheader.end();cI++){
		commonIndex.push_back(variableValues[*cI]);
		//commonIndex.push_back(VNameValuesMap[*cI]);
	    }

	    vector<SparseEntry> entries = stateTable->getSparseEntries(commonIndex);

	    //create a new end state struct for first entry
	    if(endStates.empty()){
		EndState es;
		es.endStateX = 0;
		es.endStateY = 0;
		es.probX = 1;
		es.probY = 1;
		endStates.push_back(es);
	    }
	    vector<EndState> temp = endStates;
	    int lastSize = temp.size();	//last size of end states vector before repeating

	    //multiple the number of end state by number of entries in this row 
	    //repeat the possible end states if there are more than 1 entries for this row in this table
	    for(int i=1;i<entries.size();i++){
		endStates.insert(endStates.end(), temp.begin(), temp.end());
	    }

	    //merging each entry in this table into existing end states
	    for(int i=0;i<entries.size();i++){
		SparseEntry se = entries[i];
		double probXmul, probYmul;		//probability multiplier for this entry
		int endStateXinc, endStateYinc;		//state number increment for this entry
		probXmul= probYmul= 1;
		endStateXinc = endStateYinc = 0; 

		//merge the end state X and Y
		for (unsigned int j = 0; j < se.uniqueIndex.size(); j++) {

		    StateObsAct* soa = mymap[stateTable->uIheader[j]];
		    State* s = static_cast<State*> (soa);

		    if (s->getObserved()) {
			endStateXinc += se.uniqueIndex[j].index * positionXStringIndexMap[stateTable->uIheader[j]];
			probXmul *= se.uniqueIndex[j].value;
		    } else {
			endStateYinc += se.uniqueIndex[j].index * positionYStringIndexMap[stateTable->uIheader[j]];
			probYmul *= se.uniqueIndex[j].value;
		    }
		}//end of for


		//merging increment and multiple from this entry into exisitng end states
		for(int k=i*lastSize;k<(i+1)*lastSize;k++){
		    endStates[k].endStateX += endStateXinc;
		    endStates[k].endStateY += endStateYinc;
		    endStates[k].probX *= probXmul;
		    endStates[k].probY *= probYmul;
		}
	    }

	}

	for(vector<EndState>::iterator es=endStates.begin();es!=endStates.end();es++){
	    // TXtr[a][x](x',y),
	    cOstXTrPre[action][startStateX].addEntries(es->endStateX, startStateY, es->probX);
	    // TX[a][x](y,x')
	    cOstXPre[action][startStateX].addEntries(startStateY, es->endStateX, es->probX);
	    //TYtr[a][x](y',y)
	    cOstYTrPre[action][startStateX].addEntries(es->endStateY, startStateY, es->probY);
	    //TY[a][x](y,y')
	    cOstYPre[action][startStateX].addEntries(startStateY, es->endStateY, es->probY);

	    // for checking terminal states
	    if ((es->endStateX == startStateX) && (fabs(es->probX - 1) < 0.000001))
		terminalStatesX[es->endStateX]++;
	    if ((es->endStateY == startStateY) && (fabs(es->probY - 1) < 0.000001))
		terminalStatesY[es->endStateY]++;
	    //end for checking terminal states
	}
    }while(getNextActionXYVarValues(variableValues,action,startStateX,startStateY));
    //}}}	

    //TX[a][x](y,x')
vvSparseMatrix cOstX = helperPreSparseMatrixToSparseMatrix(cOstXPre);
// printSparseMatrix("cOstX ( TX[a][x](y,x') ), action: ", cOstX, debugfile);
if (DEBUGFACTOREDCONVERSIONSTATE) {
    for (unsigned int i = 0; i < cOstX.size(); i++) {
	for (unsigned int j = 0; j < cOstX[0].size(); j++) {
	    debugfile << "cOstX ( TX[a][x](y,x') ), action: " << i
		<< " x: " << j << endl;
	    cOstX[i][j]->write(debugfile);
	    debugfile << endl;
	}
    }
}//end of if

// TXtr[a][x](x',y)
vvSparseMatrix cOstXTr = helperPreSparseMatrixToSparseMatrix(cOstXTrPre);
printSparseMatrix("cOstXTr ( TXtr[a][x](x',y) ), action: ", cOstXTr, debugfile);

//TY[a][x](y,y')
vvSparseMatrix cOstY = helperPreSparseMatrixToSparseMatrix(cOstYPre);
printSparseMatrix("cOstY ( TY[a][x](y,y') ), action: ", cOstY, debugfile);


//TYtr[a][x](y',y)
vvSparseMatrix cOstYTr = helperPreSparseMatrixToSparseMatrix(cOstYTrPre);
printSparseMatrix("cOstYTr ( TYtr[a][x](y',y) ), action: ", cOstYTr, debugfile);

// for checking terminal states
vector<int> termStatesX;
for (int i = 0; i < numMergedStatesX; i++) {
    // the number should correspond to all the number of entries
    if (terminalStatesX[i] == numActions * numMergedStatesY)
	termStatesX.push_back(1);
    else
	termStatesX.push_back(0);
}

vector<int> termStatesY;
    for (int i = 0; i < numMergedStatesY; i++) {
	if (terminalStatesY[i] == numActions * numMergedStatesX)
	    termStatesY.push_back(1);
	else
	    termStatesY.push_back(0);
    }

if (DEBUGFACTOREDCONVERSIONSTATE) {
    debugfile << "terminal states X" << endl;
    for (unsigned int i = 0; i < termStatesX.size(); i++) {
	debugfile << termStatesX[i] << " ";
    }

    debugfile << "\nterminal states Y" << endl;
    for (unsigned int i = 0; i < termStatesY.size(); i++) {
	debugfile << termStatesY[i] << " ";
    }

}

layer.TX = cOstX;
layer.TXtr = cOstXTr;
layer.TY = cOstY;
layer.TYtr = cOstYTr;

layer.isPOMDPTerminalState.push_back(termStatesX);
layer.isPOMDPTerminalState.push_back(termStatesY);

debugfile.close();
delete [] terminalStatesX;
delete [] terminalStatesY;

}//end of void FactoredPomdp::convertFactoredStateTrans()


void FactoredPomdp::convertFactoredObsTrans()
{

    ofstream debugfile;
    if (DEBUGFACTOREDCONVERSIONOBS) {
	debugfile.open("debug_FactoredPomdp_convertFactoredObs.txt");
    }

    mapObservationsToValue();

    // Conversion to the old matrices
    // Otr[a][x'](o,y')
    vvPreSparseMatrix cOobsTrPre = createVvPreSparseMatrix(numActions, numMergedStatesX, numObservations, numMergedStatesY);
    // O[a][x'](y',o)
    vvPreSparseMatrix cOobsPre = createVvPreSparseMatrix(numActions, numMergedStatesX, numMergedStatesY, numObservations);
    // end of creating matrices

    int action;
    int endStateX;
    int endStateY;
    int observ;
    //enumerate thru all the variable values
    map<string, int> variableValues = getStartActionXYVarValues();
    action = endStateX = endStateY = 0;
    do{  
	//merging the observations here
	vector<IndexProbTuple> observations; 
	for(vector<Function>::iterator obsFunc=observFunctionList.begin();obsFunc!=observFunctionList.end();obsFunc++){
	    //look up in each observation table, and merge the lookup results into index and probability

	    SharedPointer<SparseTable> obsTable = obsFunc->sparseT;
	    //assemble commom index for this table using current variable values
	    vector<int> commonIndex;
	    for(vector<string>::iterator cI=obsTable->cIheader.begin();cI!=obsTable->cIheader.end();cI++){
		commonIndex.push_back(variableValues[*cI]);
		//commonIndex.push_back(VNameValuesMap[*cI]);
	    }
	    vector<SparseEntry> entries = obsTable->getSparseEntries(commonIndex);

	    //create a new end state struct for first entry
	    if(observations.empty()){
		IndexProbTuple pt;
		pt.index = 0;
		pt.prob = 1;
		observations.push_back(pt);
	    }
	    vector<IndexProbTuple> temp = observations;
	    int lastSize = temp.size();	//last size of end states vector before repeating

	    //multiple the number of end state by number of entries in this row 
	    //repeat the possible end states if there are more than 1 entries for this row in this table
	    for(int i=1;i<entries.size();i++){
		observations.insert(observations.end(), temp.begin(), temp.end());
	    }

	    //merging each entry in this table into existing end states
	    for(int i=0;i<entries.size();i++){
		SparseEntry se = entries[i];
		double probMul=1;		//probability multiplier for this entry
		int obsInc=0;		//obseravtion number increment for this entry

		//merge the end state X and Y
		for (unsigned int j = 0; j < se.uniqueIndex.size(); j++) {
		    StateObsAct* soa = mymap[obsTable->uIheader[j]];
		    ObsAct* s = static_cast<ObsAct*> (soa);
		    obsInc += se.uniqueIndex[j].index * observationStringIndexMap[obsTable->uIheader[j]];
		    probMul *= se.uniqueIndex[j].value;
		}//end of for

		//merging increment and multiple from this entry into exisitng end states
		for(int k=i*lastSize;k<(i+1)*lastSize;k++){
		    observations[k].index += obsInc;
		    observations[k].prob *= probMul;
		}
	    }
	}
	for(vector<IndexProbTuple>::iterator pt=observations.begin();pt!=observations.end();pt++){
	    // O[a][x'](y',o)
	    cOobsPre[action][endStateX].addEntries(endStateY, pt->index, pt->prob);
	    // Otr[a][x'](o,y')
	    cOobsTrPre[action][endStateX].addEntries(pt->index, endStateY, pt->prob);
	}

    }while(getNextActionXYVarValues(variableValues,action,endStateX,endStateY));

    //- O[a][x'](y',o)
    vvSparseMatrix cOobs = helperPreSparseMatrixToSparseMatrix(cOobsPre);

    if (DEBUGFACTOREDCONVERSIONOBS) {
	for (unsigned int i = 0; i < cOobs.size(); i++) {
	    for (unsigned int j = 0; j < cOobs[0].size(); j++) {
		debugfile << "cOobs ( O[a][x'](y',o) ), action: " << i
		    << " x' : " << j << endl;
		cOobs[i][j]->write(debugfile);
		debugfile << endl;
	    }
	}
    }//end of if

    // Otr[a][x'](o,y')
    vvSparseMatrix cOobsTr = helperPreSparseMatrixToSparseMatrix(cOobsTrPre);

    if (DEBUGFACTOREDCONVERSIONOBS) {
	for (unsigned int i = 0; i < cOobsTr.size(); i++) {
	    for (unsigned int j = 0; j < cOobsTr[0].size(); j++) {
		debugfile << "cOobsTr ( Otr[a][x'](o,y') ), action: " << i
		    << " x' : " << j << endl;
		cOobsTr[i][j]->write(debugfile);
		debugfile << endl;
	    }
	}
    }//end of if

    layer.O = cOobs;
    layer.Otr = cOobsTr;
    debugfile.close();
}//end of convertFactoredObsTrans()


void FactoredPomdp::convertFactoredNoObservationsVariables() {

    numObservations = 1;

    ofstream debugfile;
    if (DEBUGFACTOREDCONVERSIONOBS) {
	debugfile.open("debug_FactoredPomdp_convertFactoredObs.txt");
    }
    debugfile << "convertFactoredNoObservationsVariables()" << endl;

    // Conversion to the old matrices
    // Otr[a][x'](o,y')
    vvPreSparseMatrix cOobsTrPre = createVvPreSparseMatrix(numActions, numMergedStatesX, 1, numMergedStatesY);
    // O[a][x'](y',o)
    vvPreSparseMatrix cOobsPre = createVvPreSparseMatrix(numActions, numMergedStatesX, numMergedStatesY, 1);
    // end of creating matrices


    for (unsigned int i = 0; i < numActions; i++) {
	for (unsigned int j = 0 ; j < numMergedStatesX; j++) {
	    for (unsigned int k = 0; k < numMergedStatesY; k++) {
		// O[a][x'](y',o)
		cOobsPre[i][j].addEntries(k, 0, 1.0);
		// Otr[a][x'](o,y')
		cOobsTrPre[i][j].addEntries(0, k, 1.0);
	    }
	}
    }

    // O[a][x'](y',o)
    vvSparseMatrix cOobs = helperPreSparseMatrixToSparseMatrix(cOobsPre);

    if (DEBUGFACTOREDCONVERSIONOBS) {
	for (unsigned int i = 0; i < cOobs.size(); i++) {
	    for (unsigned int j = 0; j < cOobs[0].size(); j++) {
		debugfile << "cOobs ( O[a][x'](y',o) ), action: " << i
		    << " x' : " << j << endl;
		cOobs[i][j]->write(debugfile);
		debugfile << endl;
	    }
	}
    }//end of if

    // Otr[a][x'](o,y')
    vvSparseMatrix cOobsTr = helperPreSparseMatrixToSparseMatrix(cOobsTrPre);

    if (DEBUGFACTOREDCONVERSIONOBS) {
	for (unsigned int i = 0; i < cOobsTr.size(); i++) {
	    for (unsigned int j = 0; j < cOobsTr[0].size(); j++) {
		debugfile << "cOobsTr ( Otr[a][x'](o,y') ), action: " << i
		    << " x' : " << j << endl;
		cOobsTr[i][j]->write(debugfile);
		debugfile << endl;
	    }
	}
    }//end of if

    layer.O = cOobs;
    layer.Otr = cOobsTr;
    debugfile.close();

}//end of convertFactoredNoObservationsVariables()



vvSparseMatrix FactoredPomdp::helperPreSparseMatrixToSparseMatrix(vvPreSparseMatrix precm) 
{
    vvSparseMatrix results;
    for (unsigned int i = 0; i < precm.size(); i++) 
    {
	vector<SharedPointer<SparseMatrix> > temp;
	for (unsigned int j = 0; j < precm[i].size(); j++) 
	{
	    temp.push_back(precm[i][j].convertSparseMatrix());
	}
	results.push_back(temp);
    }//end of for
    return results;
}

vector<vvSparseMatrix > FactoredPomdp::helperPreSparseMatrixToSparseMatrix(vector<vvPreSparseMatrix >precm) 
{
    vector<vvSparseMatrix > results;
    for (unsigned int i = 0; i < precm.size(); i++) {
	vvSparseMatrix temp_vec;
	for (unsigned int j = 0; j < precm[0].size(); j++) {
	    vector<SharedPointer<SparseMatrix> > temp;
	    for(unsigned int k=0 ; k < precm[0][0].size(); k++) {
		temp.push_back(precm[i][j][k].convertSparseMatrix());
	    }
	    temp_vec.push_back(temp);
	}  
	results.push_back(temp_vec);
    }//end of for
    return results;
}

void FactoredPomdp::convertFactoredRewardTrans() {

    ofstream debugfile;
    if (DEBUGFACTOREDCONVERSIONREWARD) {
	debugfile.open("debug_FactoredPomdp_convertFactoredReward.txt");
    }

    if (!checkRewardFunctionHasOnlyPreviousTimeSliceAndAction()) {
	preprocessRewardFunction();
    }

    // Conversion to the old matrices
    //  R[x] (y,a)
    vector<PreSparseMatrix> cOrewardPre;
    for (int i = 0; i < numMergedStatesX; i++) {
	PreSparseMatrix cm(numMergedStatesY, numActions);
	cOrewardPre.push_back(cm);
    }

    int action;
    int startStateX;
    int startStateY;
    double reward; 
    map<string, int> variableValues = getStartActionXYVarValues();
    action = startStateX = startStateY = 0;
    do{  
	//vector storing reward from individual tables to be merged
	double reward = 0;
	for(vector<Function>::iterator rewardFunc=rewardFunctionList.begin();rewardFunc!=rewardFunctionList.end();rewardFunc++){
	    //look up in each reward table, and merge the lookup results into reward 
	    SharedPointer<SparseTable> rewardTable = rewardFunc->sparseT;
	    //assemble commom index for this table using current variable values
	    vector<int> commonIndex;
	    for(vector<string>::iterator cI=rewardTable->cIheader.begin();cI!=rewardTable->cIheader.end();cI++){
		commonIndex.push_back(variableValues[*cI]);
	    }
	    vector<SparseEntry>& entries = rewardTable->getSparseEntries(commonIndex);
	    //merging each entry in this table into existing reward
	    for(int i=0;i<entries.size();i++){
		SparseEntry se = entries[i];

		//merge the end state X and Y
		for (unsigned int j = 0; j < se.uniqueIndex.size(); j++) {
		    reward += se.uniqueIndex[j].value;
		}//end of for
	    }
	}
	if (reward != 0) {
	    cOrewardPre[startStateX].addEntries(startStateY,action, reward);
	}
    }while(getNextActionXYVarValues(variableValues,action,startStateX,startStateY));

    vector<SharedPointer<SparseMatrix> > cOreward;
    for (unsigned int i = 0; i < cOrewardPre.size(); i++) {
	cOreward.push_back(cOrewardPre[i].convertSparseMatrix());
	if (DEBUGFACTOREDCONVERSIONREWARD) {
	    debugfile << "R[x] (y,a): startx " << i << endl;
	    cOreward[i]->write(debugfile);
	    debugfile << endl;
	}
    }
    layer.R = cOreward;
    debugfile.close();
}//end of convertFactoredRewardTrans()

void FactoredPomdp::convertFactoredTerminalStateReward() 
{
    ofstream debugfile;
    if (DEBUGFACTOREDCONVERSIONTERMINAL) {
	debugfile.open("debug_FactoredPomdp_convertFactoredTerminalStateReward.txt");
    }
    PreSparseMatrix terminalPre(numMergedStatesX, numMergedStatesY);

    int startStateX;
    int startStateY;
    startStateX = startStateY = 0;
    double reward;
    map<string, int> variableValues = getStartXYVarValues();
    SparseEntry se;
    do{
	bool exist = false;	//check if there is any entry in the tables
	//vector storing reward from individual tables to be merged
	double reward = 0;
	for(vector<Function>::iterator tRewardFunc=terminalStateRewardFunctionList.begin();tRewardFunc!=terminalStateRewardFunctionList.end();tRewardFunc++){
	    //look up in each state table, and merge the lookup results into endStateX and endStateY

	    SharedPointer<SparseTable> tRewardTable = tRewardFunc->sparseT;
	    //assemble commom index for this table using current variable values
	    vector<int> commonIndex;
	    for(vector<string>::iterator cI=tRewardTable->cIheader.begin();cI!=tRewardTable->cIheader.end();cI++){
		commonIndex.push_back(variableValues[*cI]);
	    }
	    vector<SparseEntry>& entries = tRewardTable->getSparseEntries(commonIndex);
	    //merging each entry in this table into existing reward
	    for(int i=0;i<entries.size();i++){
		SparseEntry se = entries[i];
		exist = true;
		//merge the end state X and Y
		for (unsigned int j = 0; j < se.uniqueIndex.size(); j++) {
		    reward += se.uniqueIndex[j].value;
		}//end of for
	    }
	}
	//  R(x,y)
	if(exist){
	    terminalPre.addEntries(startStateX,startStateY, reward);
	}
    }while(getNextXYVarValues(variableValues, startStateX, startStateY));

    SharedPointer<SparseMatrix>  terminalReward;

    terminalReward = terminalPre.convertSparseMatrix();
    if (DEBUGFACTOREDCONVERSIONTERMINAL) {
	debugfile << "R(x, y)" << endl;
	terminalReward->write(debugfile);
	debugfile << endl;
    }

    layer.terminalStateReward = terminalReward;
    debugfile.close();

}//end of convertFactoredTerminalStateReward()

void FactoredPomdp::convertFactoredBeliefCommon(ofstream& debugfile, bool printDebugFile) {
    // combine all the different state tables to form a big one
    preprocessBeliefTables(debugfile, DEBUGFACTOREDCONVERSIONSTATE);
    mergeBeliefTables(debugfile, DEBUGFACTOREDCONVERSIONSTATE);

    // remove redundant entries and entries with 0
    mapFactoredBeliefIndexesToValue(finalBeliefTable);

}//end of convertfactoredbeliefcommon

void FactoredPomdp::convertFactoredBelief() {

    ofstream debugfile;  
    if (DEBUGFACTOREDCONVERSIONBELIEF)
	debugfile.open("debug_FactoredPomdp_convertFactoredBelief.txt");
    if (DEBUGFACTOREDCONVERSIONBELIEF)
	debugfile << "Converting Factored Belief State" << endl;

    convertFactoredBeliefCommon(debugfile, DEBUGFACTOREDCONVERSIONBELIEF);

    SparseVector cvX(numMergedStatesX);
    vector<double> vec_cvX(numMergedStatesX, 0.0);
    SparseVector cvY(numMergedStatesY);
    vector<double> vec_cvY(numMergedStatesY, 0.0);

    int startX, startY, endX, endY;
    double probX, probY;

    SparseEntry se;
    while(finalBeliefTable->getNext(se)){
	startX = startY = endX = endY = 0;
	probX = probY = 1.0;

	vector<int> commonIndex = finalBeliefTable->getIterPosition();
	//start from 0
	for (unsigned int j = 0; j < commonIndex.size(); j++) {

	    StateObsAct* soa = mymap[finalBeliefTable->cIheader[j]];
	    State* s = static_cast<State*> (soa);

	    if (s->getObserved())
		startX += commonIndex[j] * factoredPositionCIIndexMap[j];
	    else
		startY += commonIndex[j] * factoredPositionCIIndexMap[j];
	}


	for (unsigned int j = 0; j < se.uniqueIndex.size(); j++) {

	    StateObsAct* soa = mymap[finalBeliefTable->uIheader[j]];
	    State* s = static_cast<State*> (soa);

	    if (s->getObserved()) {
		endX += se.uniqueIndex[j].index * factoredPositionUIIndexMap[j];
		probX *= se.uniqueIndex[j].value;
	    } else {
		endY += se.uniqueIndex[j].index * factoredPositionUIIndexMap[j];
		probY *= se.uniqueIndex[j].value;
	    }
	}//end of for

	if (startX != endX) {
	    cerr << "for initial belief state, the entries should have common indexes correspoding to unique indexes" << endl;
	    assert(false);
	}
	if (startY != endY) {
	    cerr << "for initial belief state, the entries should have common indexes correspoding to unique indexes" << endl;
	    assert(false);
	}

	vec_cvX[startX] = probX;
	vec_cvY[startY] = probY;

    }

    int numRandomPositions = 0;
    for (unsigned int i=0 ; i < vec_cvX.size(); i++) {
	if (fabs(vec_cvX[i]) > 0.000001) {
	    layer.initialStateX = i;
	    numRandomPositions++;
	    cvX.push_back(i, vec_cvX[i]);
	}
    }

    for (unsigned int i=0 ; i < vec_cvY.size(); i++) {
	if (fabs(vec_cvY[i]) > 0.000001)    
	    cvY.push_back(i, vec_cvY[i]);
    }

    if (numRandomPositions > 1)
	layer.initialStateX = -1;

    layer.initialBeliefX = cvX;
    layer.initialBeliefY = cvY;


    if (DEBUGFACTOREDCONVERSIONBELIEF) {
	debugfile << "inital state X: " << layer.initialStateX << endl;
	debugfile << "initial belief X " << endl;
	layer.initialBeliefX.write(debugfile);
	debugfile << "\ninitial belief Y " << endl;
	layer.initialBeliefY.write(debugfile);
    }
    debugfile << endl;
    debugfile.close();

}//end of convertfactoredbelief


void FactoredPomdp::convertFactoredBeliefReparam() {

    ofstream debugfile;  
    if (DEBUGFACTOREDCONVERSIONBELIEF)
	debugfile.open("debug_FactoredPomdp_convertFactoredBelief.txt");
    if (DEBUGFACTOREDCONVERSIONBELIEF)
	debugfile << "Converting Factored Belief State REPARAM" << endl;

    convertFactoredBeliefCommon(debugfile, DEBUGFACTOREDCONVERSIONBELIEF);

    SparseVector cvX(numMergedStatesX);
    vector<double> vec_cvX(numMergedStatesX, 0.0);

    vector<SparseVector> cvY;
    for (unsigned int i=0; i < numMergedStatesX; i++) {
	SparseVector cv(numMergedStatesY);
	cvY.push_back(cv);
    }
    vector<vector<double> > vec_cvY;
    for (unsigned int i=0; i < numMergedStatesX; i++) {
	vector<double> vec_cv(numMergedStatesY,0.0);
	vec_cvY.push_back(vec_cv);
    }


    int startX, startY, endX, endY;
    double probX, probY;
    SparseEntry se;
    while(finalBeliefTable->getNext(se)){
	startX = startY = endX = endY = 0;
	probX = probY = 1.0;
	vector<int> commonIndex = finalBeliefTable->getIterPosition();

	//start from 0
	for (unsigned int j = 0; j < commonIndex.size(); j++) {

	    StateObsAct* soa = mymap[finalBeliefTable->cIheader[j]];
	    State* s = static_cast<State*> (soa);

	    if (s->getObserved())
		startX += commonIndex[j] * factoredPositionCIIndexMap[j];
	    else
		startY += commonIndex[j] * factoredPositionCIIndexMap[j];
	}


	for (unsigned int j = 0; j < se.uniqueIndex.size(); j++) {

	    StateObsAct* soa = mymap[finalBeliefTable->uIheader[j]];
	    State* s = static_cast<State*> (soa);

	    if (s->getObserved()) {
		endX += se.uniqueIndex[j].index * factoredPositionUIIndexMap[j];
		probX *= se.uniqueIndex[j].value;
	    } else {
		endY += se.uniqueIndex[j].index * factoredPositionUIIndexMap[j];
		probY *= se.uniqueIndex[j].value;
	    }
	}//end of for

	if (startX != endX) {
	    cerr << "for initial belief state, the entries should have common indexes correspoding to unique indexes" << endl;
	    assert(false);
	}
	if (startY != endY) {
	    cerr << "for initial belief state, the entries should have common indexes correspoding to unique indexes" << endl;
	    assert(false);
	}

	vec_cvX[startX] = probX;
	vec_cvY[startX][startY] = probY;
    }//end of for
    finalBeliefTable = NULL;

    int numRandomPositions = 0;
    for (unsigned int i=0 ; i < vec_cvX.size(); i++) {
	if (fabs(vec_cvX[i]) > 0.000001) {
	    layer.initialStateX = i;
	    numRandomPositions++;
	    cvX.push_back(i, vec_cvX[i]);
	}
    }

    for (unsigned int i=0 ; i < vec_cvY.size(); i++) {
	for (unsigned int j=0; j < vec_cvY[0].size(); j++) {
	    if (fabs(vec_cvY[i][j]) > 0.000001)    
		cvY[i].push_back(j, vec_cvY[i][j]);
	}
    }

    if (numRandomPositions > 1)
	layer.initialStateX = -1;

    layer.initialBeliefX = cvX;
    layer.initialBeliefY_reparam = cvY;

    if (DEBUGFACTOREDCONVERSIONBELIEF) {
	debugfile << "inital state X: " << layer.initialStateX << endl;
	debugfile << "initial belief X " << endl;
	layer.initialBeliefX.write(debugfile);
	debugfile << "\ninitial belief Y reparam" << endl;
	for (unsigned int i=0; i < layer.initialBeliefY_reparam.size(); i++) {
	    debugfile << "when x is " << i << endl;
	    layer.initialBeliefY_reparam[i].write(debugfile);
	}
    }
    debugfile << endl;
    debugfile.close();

}//end of convertfactoredbelief



void FactoredPomdp::preprocessBeliefTables(ofstream& debugfile, bool printDebugFile) 
{
    if(preprocessBeliefTablesDone)
    {
	return;
    }
    preprocessBeliefTablesDone = true;

    for (unsigned int i = 0; i < beliefFunctionList.size(); i++) {

	if (printDebugFile) {
	    debugfile << "belief function before preprocess" << i << endl;
	    beliefFunctionList[i].write(debugfile);
	    debugfile << endl;
	}
    }//end of for

    // clear it, this is merely for convenience since we may call convertfast after convertfactored during testing
    processedBeliefFunctionList.clear();
    for(unsigned int i=0; i < beliefFunctionList.size(); i++) {
	processedBeliefFunctionList.push_back(beliefFunctionList[i]);
    }

    for (unsigned int i = 0; i < processedBeliefFunctionList.size(); i++) {

	vector<string> parents = processedBeliefFunctionList[i].getParents();
	SharedPointer<SparseTable> old = processedBeliefFunctionList[i].sparseT;
	vector<string> newParent;
	vector<int> newNumCIValues; 
	if (!((parents.size() == 1) && (parents[0] == "null"))) { 
	    //parent is not null
	    newParent = processedBeliefFunctionList[i].getParents();
	    newNumCIValues = old->numCIValues; //only copy if there is non-null parent
	}
	newParent.push_back(processedBeliefFunctionList[i].getVNameCurr());
	processedBeliefFunctionList[i].setParents(newParent);

	//"copy" unique index to common index
	//which in fact transpose the table
	//
	newNumCIValues.insert(newNumCIValues.end(), old->numUIValues.begin(), old->numUIValues.end());
		processedBeliefFunctionList[i].sparseT = SharedPointer<SparseTable> (new SparseTable(newParent, old->uIheader, newNumCIValues, old->numUIValues));

	old->sortEntries();
	SparseEntry se;
	old->resetIterator();
	vector<int> newCI;
	while(old->getNext(se)){
	    newCI = old->getIterPosition(); 	//copy original CI if parent is not null
	    if( (parents.size() == 1) && (parents[0] == "null") ){ 
		newCI.clear(); 	//don't use original CI if parent is null
	    }
	    newCI.push_back(se.uniqueIndex[0].index);
	    processedBeliefFunctionList[i].sparseT->add(newCI, se);
	}
    }

    for (unsigned int i = 0; i < processedBeliefFunctionList.size(); i++) {

	if (printDebugFile) {
	    debugfile << "belief function after preprocess" << i << endl;
	    processedBeliefFunctionList[i].write(debugfile);
	    debugfile << endl;
	}
    }//end of for

}

void FactoredPomdp::mergeBeliefTables(ofstream& debugfile, bool printDebugFile) {

    unsigned int pos;

    finalBeliefTable = processedBeliefFunctionList[0].sparseT;
    SharedPointer<SparseTable>  finalBeliefTable2;
    for (unsigned int i = 1; i < processedBeliefFunctionList.size(); i++) {

	finalBeliefTable = SparseTable::join(*finalBeliefTable,
		*processedBeliefFunctionList[i].sparseT, BELIEFFUNCTION);

	if (printDebugFile) {
	    debugfile << "Intermediate finalBeliefTable " << i << endl;
	    finalBeliefTable->write(debugfile);
	    debugfile << endl;
	}

    }//end of for

}//end of mergeBeliefTables


void FactoredPomdp::convertFactoredVariables() {

    ofstream debugfile;
    if (DEBUGFACTOREDCONVERSIONOTHERS)
	debugfile.open("debug_FactoredPomdp_convertFactoredOthers.txt");

    layer.discount = discount;
    if (DEBUGFACTOREDCONVERSIONOTHERS)
	debugfile << "\ndiscount: " << layer.discount << endl;

    layer.numActions = numActions;
    if (DEBUGFACTOREDCONVERSIONOTHERS)
	debugfile << "num actions: " << layer.numActions << endl;

    layer.numObservations = numObservations;
    if (DEBUGFACTOREDCONVERSIONOTHERS)
	debugfile << "num observations: " << layer.numObservations << endl;

    layer.numStatesObs = 1;
    layer.numStatesUnobs = 1;

    for (unsigned int i = 0; i < stateList.size(); i++) {
	if (stateList[i].getObserved())
	    layer.numStatesObs *= stateList[i].getValueEnum().size();
	else
	    layer.numStatesUnobs *= stateList[i].getValueEnum().size();
    }

    if (DEBUGFACTOREDCONVERSIONOTHERS)
	debugfile << "num states obs: " << layer.numStatesObs << endl;
    if (DEBUGFACTOREDCONVERSIONOTHERS)
	debugfile << "num states unobs: " << layer.numStatesUnobs << endl;

    debugfile.close();
}//end of FactoredPomdp::convertFactoredVariables


SharedPointer<SparseTable> FactoredPomdp::expandObsRewSparseTable(SharedPointer<SparseTable> st, int whichFunction) {

    //currentcoding
    //expand function to fit the size of finalStateTable

    vector<string> canonicalValues;
    if (whichFunction == OBSERVFUNCTION) {
	canonicalValues = canonicalNameCurr;
    }else if (whichFunction == REWARDFUNCTION) {
	canonicalValues = canonicalNamePrev;
    }else if (whichFunction == TERMINALFUNCTION) {
	canonicalValues = canonicalNameForTerminal;
    }else {
	assert(false);
    }

    vector<string> cIheader, uIheader;
    vector<int> numUIValues, numCIValues;
    cIheader = st->cIheader;
    uIheader = st->uIheader;
    numUIValues = st->numUIValues;
    numCIValues = st->numCIValues;

    for (unsigned int i = 0; i < canonicalValues.size() ; i++) {
	if (!(st->containsCI(canonicalValues[i]))) {
	    cIheader.push_back(canonicalValues[i]);
	    StateObsAct* soa = mymap[canonicalValues[i]];
	    numCIValues.push_back(soa->getValueEnum().size());

	}//end of if
    }//end of for

    SharedPointer<SparseTable> newSt (new SparseTable(cIheader, uIheader, numCIValues, numUIValues));

    SparseEntry se; 
    //
    //expand table

    vector<int> newStPos = newSt->getIterBegin();
    bool hasNext = true;
    while(hasNext){ 
	//get the portion old CI from front portion of the new CI
	vector<int> oldStPos;
	for(int i=0;i<st->cIheader.size();i++){
	    oldStPos.push_back(newStPos[i]);
	}
	vector<SparseEntry> entries = st->getSparseEntries(oldStPos);
	//add it into new table, it may be repeated
	for(int i=0;i<entries.size();i++){
	    newSt->add(newStPos, entries[i]);
	}

	hasNext = newSt->getNextCI(newStPos);
    }

    return newSt;
}//end of expandObsRewSparseTable



//////////////////////////////////////////////////////////////
// for conversion of R(x,y,x',y',a,o) to R(x,y,a)

void FactoredPomdp::preprocessRewardFunction() 
{
    ofstream debugfile;
    if (DEBUGFACTOREDCONVERSIONREWARD) {
	debugfile.open("debug_FactoredPomdp_preprocessRewardTable.txt");
    }

    vector<SharedPointer<SparseTable> > rewardTables;

    //preprocess each reward table individually...
    for (unsigned int i=0; i < rewardFunctionList.size(); i++) {
	debugfile << "reward function " << i <<endl;

	//determine the functions required for conversion
	//i.e functions of current time slice variable used
	vector<Function> functionsDepend;
	set<string> curTimeVars = getRewardFunctionCurrentTimeSliceVars(&rewardFunctionList[i] );
	for(set<string>::iterator it=curTimeVars.begin();it!=curTimeVars.end();it++)
	{
	    functionsDepend.push_back(*mapFunc[*it]);
	}
	functionsDepend.push_back(rewardFunctionList[i]);

	//merge reward table with the functions it depends on
	SharedPointer<SparseTable> rewardTable = mergeTables(&functionsDepend, BELIEFFUNCTION, debugfile, DEBUGFACTOREDCONVERSIONREWARD); 
	rewardTable->write(debugfile);

	debugfile << "after reducing common indexes that are not matched with unique indexes" << endl;
	rewardTable = reduceUnmatchedCIWithUI(rewardTable, debugfile, DEBUGFACTOREDCONVERSIONREWARD);
	rewardTable->write(debugfile);

	debugfile << "after removing redundant unique indexes fron reward table" << endl;
	rewardTable = removeRedundantUIsFromReward(rewardTable);
	rewardTable->write(debugfile);

	//replace the sparse table in original function with new merged sparse table
	rewardFunctionList[i].sparseT = rewardTable;
    }
}

SharedPointer<SparseTable> FactoredPomdp::removeRedundantUIsFromReward(SharedPointer<SparseTable> st) {
    //also remove redundant observations from CIs

    vector<bool> indexesToReward;
    vector<string> newUIHeader;
    vector<string> newCIHeader;
    vector<int> newNumCIValues;
    vector<int> newNumUIValues;
    for (unsigned int i=0; i < st->uIheader.size(); i++) {
	if (checkRewardNameExists(st->uIheader[i])) {
	    newUIHeader.push_back(st->uIheader[i]);
	    newNumUIValues.push_back(st->numUIValues[i]);
	    indexesToReward.push_back(true);
	}else {
	    indexesToReward.push_back(false);
	}
    }

    SharedPointer<SparseTable> st2(new SparseTable(st->cIheader, newUIHeader, st->numCIValues, newNumUIValues));

    double prob;
    SparseEntry se;
    st->resetIterator();
    while(st->getNext(se)){
	SparseEntry newSe;
	vector<int> ci = st->getIterPosition();
	prob = 1.0;
	for (unsigned int j=0; j < se.uniqueIndex.size(); j++) {
	    if (!(indexesToReward[j])) prob *= se.uniqueIndex[j].value;
	}

	for (unsigned int j=0; j < se.uniqueIndex.size(); j++) {
	    if ((indexesToReward[j])) {
		UniqueIndex uq;
		uq.index = 0;
		uq.value = se.uniqueIndex[j].value * prob;
		newSe.uniqueIndex.push_back(uq);
	    }
	}
	st2->add(ci, newSe);
    }
    return st2;
}// end of removeRedundantUIsFromReward(SparseTable st)


/////////////////////////////////////////////////////////////
//for a future memory-efficient solver
//symbolic information

map<string, string> FactoredPomdp::getActionsSymbols(int actionNum) {

    cout << "getActionsSymbols" << endl;

    map<string, string> result;

    int quotient, remainder;
    quotient = actionNum;
    for (int i = (int) actionList.size() - 1; i >= 0; i--) {

	ObsAct act = actionList[i];

	remainder = quotient % act.getValueEnum().size();
	result[act.getVName()] = act.getValueEnum()[remainder];
	quotient = quotient / act.getValueEnum().size();

    }
    return result;
}


map<string, string> FactoredPomdp::getFactoredObservedStatesSymbols(int stateNum) {

    cout << "getFactoredObservedStatesSymbols" << endl;

    map<string, string> result;

    int quotient, remainder;
    quotient = stateNum;
    for (int i = (int) stateList.size() - 1; i >= 0; i--) {

	State s = stateList[i];
	if (s.getObserved()) {

	    remainder = quotient % s.getValueEnum().size();
	    result[s.getVNamePrev()] = s.getValueEnum()[remainder];
	    result[s.getVNameCurr()] = s.getValueEnum()[remainder];
	    quotient = quotient / s.getValueEnum().size();
	}
    }
    return result;
}


map<string, string> FactoredPomdp::getFactoredUnobservedStatesSymbols(int stateNum) {

    cout << "getFactoredUnObservedStatesSymbols" << endl;

    map<string, string> result;

    int quotient, remainder;
    quotient = stateNum;
    for (int i = (int) stateList.size() - 1; i >= 0; i--) {

	State s = stateList[i];
	if (!(s.getObserved())) {

	    remainder = quotient % s.getValueEnum().size();
	    result[s.getVNamePrev()] = s.getValueEnum()[remainder];
	    result[s.getVNameCurr()] = s.getValueEnum()[remainder];
	    quotient = quotient / s.getValueEnum().size();
	}
    }
    return result;
}

map<string, string> FactoredPomdp::getObservationsSymbols(int observationNum) {

    cout << "getObservationsSymbols" << endl;

    map<string, string> result;

    int quotient, remainder;
    quotient = observationNum;
    for (int i = (int) observationList.size() - 1; i >= 0; i--) {

	ObsAct obs = observationList[i];

	remainder = quotient % obs.getValueEnum().size();
	result[obs.getVName()] = obs.getValueEnum()[remainder];
	quotient = quotient / obs.getValueEnum().size();

    }
    return result;
}

//get the first set of values for action, X, Y variables
//all zero
map<string, int> FactoredPomdp::getStartActionXYVarValues(){
    map<string, int> varValues = getStartXYVarValues();
    for(int i=0;i<actionList.size();i++){
	varValues[actionList[i].getVName()] = 0;
    }
    return varValues;
}

map<string, int> FactoredPomdp::getStartXYVarValues(){
    map<string, int> varValues;
    for(int i=0;i<stateList.size();i++){
	varValues[stateList[i].getVNamePrev()] = 0;
	varValues[stateList[i].getVNameCurr()] = 0;
    }
    return varValues;
}

//get the first set of values for action, S variables
//all zero
map<string, int> FactoredPomdp::getStartActionSVarValues(){
    map<string, int> varValues = getStartSVarValues();
    for(int i=0;i<actionList.size();i++){
	varValues[actionList[i].getVName()] = 0;
    }
    return varValues;
}

map<string, int> FactoredPomdp::getStartSVarValues(){
    map<string, int> varValues;
    for(int i=0;i<stateList.size();i++){
	varValues[stateList[i].getVNamePrev()] = 0;
	varValues[stateList[i].getVNameCurr()] = 0;
    }
    return varValues;
}

//get the next values for action, X, Y variables
bool FactoredPomdp::getNextActionXYVarValues(map<string, int> &curValues, int &action, int &stateX, int &stateY){

    if(getNextXYVarValues(curValues, stateX, stateY)){
	return true;
    }
    //try to increment the action
    for(int i=actionList.size()-1;i>=0;i--){
	ObsAct act = actionList[i];
	if(curValues[act.getVName()] >= act.getValueEnum().size() -1){
	    //at last value of this variable, reset to 0 and try to increment next variable
	    curValues[act.getVName()]=0;  
	}
	else{
	    curValues[act.getVName()]++;
	    action++;
	    return true;
	}
    }
    action=0;
    //reach here if we are at the last value of action
    return false;
}

//get the next values for action, X, X', Y variables -- used by convertFactoredReparam
bool FactoredPomdp::getNextActionXXpYVarValues(map<string, int> &curValues, int &action, int &stateX, int &stateXp, int &stateY){
    //try to increment unobserved state first
    for(int i=stateList.size()-1;i>=0;i--){
	State s = stateList[i];
	if (!s.getObserved()) {
	    if(curValues[s.getVNamePrev()] >= s.getValueEnum().size()-1){
		//at last value of this variable, reset to 0 and try to increment next variable
		curValues[s.getVNamePrev()]=0;  
		curValues[s.getVNameCurr()]=0;  
	    }
	    else{
		curValues[s.getVNamePrev()]++;
		curValues[s.getVNameCurr()]++;
		stateY++;
		return true;
	    }
	}
    }
    //reach here if we are at the last value of state X, reset state X
    stateY=0;

    //try to increment observed state next
    for(int i=stateList.size()-1;i>=0;i--){
	State s = stateList[i];
	if (s.getObserved()) {
	    //try to increment unobserved state first
	    if(curValues[s.getVNamePrev()] >= s.getValueEnum().size() -1){
		//at last value of this variable, reset to 0 and try to increment next variable
		curValues[s.getVNamePrev()]=0;  
	    }
	    else{
		curValues[s.getVNamePrev()]++;
		stateX++;
		return true;
	    }
	}
    }
    stateX=0;

    //try to increment current observed state next
    for(int i=stateList.size()-1;i>=0;i--){
	State s = stateList[i];
	if (s.getObserved()) {
	    //try to increment unobserved state first
	    if(curValues[s.getVNameCurr()] >= s.getValueEnum().size() -1){
		//at last value of this variable, reset to 0 and try to increment next variable
		curValues[s.getVNameCurr()]=0;  
	    }
	    else{
		curValues[s.getVNameCurr()]++;
		stateXp++;
		return true;
	    }
	}
    }
    stateXp =0;

    //try to increment the action
    for(int i=actionList.size()-1;i>=0;i--){
	ObsAct act = actionList[i];
	if(curValues[act.getVName()] >= act.getValueEnum().size() -1){
	    //at last value of this variable, reset to 0 and try to increment next variable
	    curValues[act.getVName()]=0;  
	}
	else{
	    curValues[act.getVName()]++;
	    action++;
	    return true;
	}
    }
    action=0;
    //reach here if we are at the last value of action
    return false;
}


bool FactoredPomdp::getNextXYVarValues(map<string, int> &curValues, int &stateX, int &stateY){
    //try to increment unobserved state first
    for(int i=stateList.size()-1;i>=0;i--){
	const State& s = stateList[i];
	if (!s.getObserved()) {
	    if(curValues[s.getVNamePrev()] >= s.getValueEnum().size()-1){
		//at last value of this variable, reset to 0 and try to increment next variable
		curValues[s.getVNamePrev()]=0;  
		curValues[s.getVNameCurr()]=0;  
	    }
	    else{
		curValues[s.getVNamePrev()]++;
		curValues[s.getVNameCurr()]++;
		stateY++;
		return true;
	    }
	}
    }
    //reach here if we are at the last value of state X, reset state X
    stateY=0;

    //try to increment observed state next
    for(int i=stateList.size()-1;i>=0;i--){
	const State& s = stateList[i];
	if (s.getObserved()) {
	    //try to increment unobserved state first
	    if(curValues[s.getVNamePrev()] >= s.getValueEnum().size() -1){
		//at last value of this variable, reset to 0 and try to increment next variable
		curValues[s.getVNamePrev()]=0;  
		curValues[s.getVNameCurr()]=0;  
	    }
	    else{
		curValues[s.getVNamePrev()]++;
		stateX++;
		curValues[s.getVNameCurr()]++;
		return true;
	    }
	}
    }
    //reach here if we are at the last value of state Y, reset state Y
    stateX=0;
    return false;
}

//get the next values for action, S variables
bool FactoredPomdp::getNextActionSVarValues(map<string, int> &curValues, int &action, int &stateNum){

    if(getNextSVarValues(curValues, stateNum)){
	return true;
    }
    //try to increment the action
    for(int i=actionList.size()-1;i>=0;i--){
	ObsAct act = actionList[i];
	if(curValues[act.getVName()] >= act.getValueEnum().size() -1){
	    //at last value of this variable, reset to 0 and try to increment next variable
	    curValues[act.getVName()]=0;  
	}
	else{
	    curValues[act.getVName()]++;
	    action++;
	    return true;
	}
    }
    action=0;
    //reach here if we are at the last value of action
    return false;
}

//get the next values for S variables -- for convertFast
bool FactoredPomdp::getNextSVarValues(map<string, int> &curValues,int &stateNum){
    //try to increment unobserved state first
    for(int i=stateList.size()-1;i>=0;i--){
	State s = stateList[i];
	if(curValues[s.getVNamePrev()] >= s.getValueEnum().size()-1){
	    //at last value of this variable, reset to 0 and try to increment next variable
	    curValues[s.getVNamePrev()]=0;  
	    curValues[s.getVNameCurr()]=0;  
	}
	else{
	    curValues[s.getVNamePrev()]++;
	    curValues[s.getVNameCurr()]++;
	    stateNum++;
	    return true;
	}
    }
    //reach here if we are at the last value of state S, reset state S
    stateNum=0;
    return false;
}

