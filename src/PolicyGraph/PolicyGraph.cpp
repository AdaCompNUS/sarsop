#include "MOMDP.h"
#include "ParserSelector.h"
#include "AlphaVectorPolicy.h"
#include "BeliefForest.h"
#include "Sample.h"
#include "BeliefCache.h"
#include "EvaluatorSampleEngine.h"
#include "EvaluatorBeliefTreeNodeTuple.h"

#include "GlobalResource.h"
#include "PolicyGraphGenerator.h"

#include <string>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <ctime>

#include "CPTimer.h"

using namespace std;
using namespace momdp;

void print_usage(const char* cmdName) 
{
    cout << "Usage: " << cmdName << " POMDPModelFileName --policy-file policyFileName --policy-graph policyGraphFileName \n"
<<"	 [--fast] [--graph-max-depth maximumDepth] [--graph-max-branch maximumNoOfBranches]\n"
<<"         [--graph-min-prob minimumProbability]" << endl;
    cout << "    or " << cmdName << " --help (or -h)		  Print this help."<< endl;
    cout << "    or " << cmdName << " --version 		  Print version information." << endl;
    cout << endl;
    cout << "Policy graph generator options:" << endl; 
    cout << "  --policy-file policyFileName	          Use policyFileName as the policy file name (compulsory)." << endl;
    cout << "  -f or --fast			  	  Use fast (but very picky) alternate parser for .pomdp files.\n";
    cout << "  --graph-max-depth maximumDepth	  Use maximumDepth as the maximum horizon of the generated policy\n"
	 << "					  graph. There is no limit by default." << endl;
    cout << "  --graph-max-branch maximumNoOfBranches  Use maximumNoOfBranches as the maximum number of branches to show\n"
	 << "					  in the policy graph. The branches shown are the top\n"
	 << "					  maximumNofOfBranches branches in probability. There is no limit\n"
	<<  "					  by default." << endl;
    cout << "  --graph-min-prob minimumProbability	  Use minimumProbability as the minimum probability threshold for a\n"
	 << "					  branch to be shown in the policy graph. Branches with\n"
	 << "					  probability less than the threshold are suppressed. The threshold\n"
	 << " 					  is zero by default." << endl;
    cout << "" << endl;
    cout << "Output options" << endl;
    cout << "  --policy-graph policyGraphFileName  	  Use policyGraphFileName as the name for the DOT file to be \n"
	 << "					  generated (compulsory)." << endl;
    cout << "" << endl;
/*    cout << "Miscellaneous options:" << endl;
    cout << "	-h or --help                         Print help." << endl;
    cout << "	--version                            Print version information" << endl;*/
    cout <<"Example:" <<endl;
    cout << "  " << cmdName << " --policy-file hallway.policy --policy-graph Hallway.dot --graph-max-depth 5 Hallway.pomdp" << endl;
}


int main(int argc, char **argv) 
{
    try
    {
	SolverParams* p =&GlobalResource::getInstance()->solverParams;
	bool parseCorrect = SolverParams::parseCommandLineOption(argc, argv, *p);
	if(!parseCorrect)
	{
	    print_usage(p->cmdName);
	    exit(EXIT_FAILURE);
	}

	//check validity of options
	if (p->policyFile == "" || p->policyGraphFile.length() == 0) 
	{
	    print_usage(p->cmdName);
	    return 0;
	}

	SharedPointer<MOMDP> problem = ParserSelector::loadProblem(p->problemName, *p);
	SharedPointer<AlphaVectorPolicy> policy = new AlphaVectorPolicy(problem);
	bool policyRead = policy->readFromFile(p->policyFile);
	if(!policyRead)
	{
	    return 0;
	}

    if (policy->getValueAction() == -1 && !p->useLookahead) {
        cerr<<"Use the lookahead controller (--lookahead yes) for MDP policies."<<endl;
        exit(EXIT_FAILURE);
    }

	if(p->useLookahead)
	{
	    cout << "Use one-step look ahead" << endl;
	}

	PolicyGraphParam graphParam;
	graphParam.useLookahead = p->useLookahead;
	graphParam.depth = p->graphDepth;
	graphParam.probThreshold = p->graphProbThreshold;
	graphParam.maxEdge = p->graphMaxBranch;
	ofstream dotFile(p->policyGraphFile.c_str());
	dotFile.precision(3);

	PolicyGraphGenerator generator(problem, policy, graphParam);
	generator.generateGraph(dotFile);

	dotFile.flush();
	dotFile.close();
    }
    catch(bad_alloc &e)
    {
	if(GlobalResource::getInstance()->solverParams.memoryLimit == 0)
	{
	    cout << "Memory allocation failed. Exit." << endl;
	}
	else
	{
	    cout << "Memory limit reached. Please try increase memory limit" << endl;
	}

    }
    catch(exception &e)
    {
	cout << "Exception: " << e.what() << endl ;
    }

    return 0;
}

