#include "ParserSelector.h"
#include "Parser.h" //Cassandra's parser
#include "POMDP.h"

using namespace momdp;
using namespace std;

ParserSelector::ParserSelector(void)
{
}

ParserSelector::~ParserSelector(void)
{
}

SharedPointer<MOMDP> ParserSelector::loadProblem(string problemName, SolverParams& solverParam)
{
	SharedPointer<MOMDP> problem;

	// added from png shaowei's code
	ifstream myfile;
	myfile.open(problemName.c_str());
	string line;
	bool xmlfile = false;
	if (myfile.is_open()) 
	{
		getline(myfile, line);

		string first4chars = line.substr(0, 5);
		if (first4chars == "<?xml") 
		{
			xmlfile = true;
		}
	}
	else
	{
		cout << "Cannot open file: " << problemName << endl;
		exit(-1);
	}
	myfile.close();

	//FactoredPomdp test;
	if (xmlfile) 
	{
		FactoredPomdp* factoredP = new FactoredPomdp(problemName);
		unsigned int probType = factoredP->start(); // read in the file
		bool flag_copyOldPOMDP = false;
		switch (probType) 
		{
			// MOMDP Lite
		case MIXED :
			DEBUG_LOG(cout << "MIXED without reparam" << endl;);

			factoredP->convertFactored();
			if(solverParam.doConvertPOMDP)
			{
				factoredP->convertFast();
				flag_copyOldPOMDP = true;
			}

			break;

			// MOMDP proper
		case MIXED_REPARAM:
			DEBUG_LOG(cout << "mixed reparam. calling convertfactoredreparam" << endl;);
			factoredP->convertFactoredReparam();
			if(solverParam.doConvertPOMDP)
			{
				factoredP->convertFast();
				flag_copyOldPOMDP = true;
			}
			DEBUG_LOG(cout << "mixed reparam. solver part is under working." << endl;);
			// exit(0);
			break;

			// POMDP in POMDPX file format
		case FULLY_UNOBSERVED:

			DEBUG_LOG(cout << "fully unobserved " << endl;);
			factoredP->convertFast();
			break;

		case FULLY_OBSERVED:
			// MDP in this case
			solverParam.MDPSolution = true; 	//enable MDP solution
			factoredP->convertFast();
		}

		//problem = new POMDP(&(factoredP->layer), flag_copyOldPOMDP, probType);
		problem = MOMDP::convertMOMDPFromPOMDPX(factoredP, flag_copyOldPOMDP, probType);

		delete factoredP;
	} 
	else 
	{ 
		// parse the problem using cassandra's parser and write parameters into the POMDP object (problem)
		Parser* parser = new Parser();
		POMDP* pomdpProblem = parser->parse(problemName, solverParam.useFastParser);
		problem = MOMDP::convertMOMDPFromPOMDP(pomdpProblem);
	}
	return problem;
}
