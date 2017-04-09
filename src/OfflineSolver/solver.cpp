/** 
* Part of the this code is derived from ZMDP: http://www.cs.cmu.edu/~trey/zmdp/
* ZMDP is released under Apache License 2.0
* The rest of the code is released under GPL v2 
*/


#include <stdio.h>
#include <stdlib.h>
#include <cfloat>

#ifdef _MSC_VER
#include "getopt.h"
#define NOMINMAX 
#include <windows.h> 

#else
#include <getopt.h>
#include <sys/time.h>
#endif
#include <signal.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <assert.h>

#include "CPTimer.h"

#include "GlobalResource.h"
//#include "ActionSelector.h"
//#include "PolicyFollower.h"

#include "solverUtils.h"
#include "Parser.h"
#include "POMDP.h"
#include "ParserSelector.h"
#include "MOMDP.h"
#include "SARSOP.h"
#include "BackupAlphaPlaneMOMDP.h"
#include "BackupBeliefValuePairMOMDP.h"

//#include "FSVI.h"
//#include "GES.h"
#include "FullObsUBInitializer.h"
#include "FastInfUBInitializer.h"

#include <string.h>

using namespace std;
using namespace momdp;

#ifdef __cplusplus
extern "C"
#endif
{
	extern unsigned long GlobalMemLimit;
}

struct OutputParams {
	double timeoutSeconds;
	double interval;
	OutputParams(void);
};

OutputParams::OutputParams(void) {
	timeoutSeconds = -1;
	interval = -1;
}


#ifdef _MSC_VER
BOOL CtrlHandler( DWORD fdwCtrlType ) 
{ 
	switch( fdwCtrlType ) 
	{ 
		// Handle the interrupt signal. 
	case CTRL_C_EVENT: 
	case CTRL_CLOSE_EVENT: 
	case CTRL_BREAK_EVENT: 
	case CTRL_SHUTDOWN_EVENT: 
	case CTRL_LOGOFF_EVENT:
		if(GlobalResource::getInstance()->solving)
		{
			GlobalResource::getInstance()->userTerminatedG = true;
		}
		else
		{
			exit(1);
		}
		printf("*** Received SIGINT. User pressed control-C. ***\n");
		printf("\nTerminating ...\n");
		fflush(stdout);
		GlobalResource::getInstance()->userTerminatedG = true;
		return( TRUE );

	default: 
		return FALSE; 
	} 
} 

void registerCtrlHanler()
{
	if( SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, TRUE ) ) 
	{ 
		// Success
	} 
	else 
	{
		// Failed to register... but continue anyway
		printf( "\nERROR: Could not set control handler"); 
	}
}

#else

void sigIntHandler(int sig) {

	if(GlobalResource::getInstance()->solving)
	{
		GlobalResource::getInstance()->userTerminatedG = true;
	}
	else
	{
		exit(1);
	}


	printf("*** Received SIGINT. User pressed control-C. ***\n");
	printf("\nTerminating ...\n");
	fflush(stdout);
}

void setSignalHandler(int sig, void (*handler)(int)) 
{
	struct sigaction act;
	memset (&act, 0, sizeof(act));
	act.sa_handler = handler;
	act.sa_flags = SA_RESTART;
	if (-1 == sigaction (sig, &act, NULL)) {
		cerr << "ERROR: unable to set handler for signal "
			<< sig << endl;
		exit(EXIT_FAILURE);
	}


}
#endif

void usage(const char* cmdName)
{
	cerr <<
		"Usage: " << cmdName << " POMDPModelFileName [--fast] [--precison targetPrecision] [--randomization]\n" 
"	[--timeout timeLimit] [--memory memoryLimit] [--output policyFileName]\n" 
"	[--policy-interval timeInterval]\n"
		"    or " <<cmdName << " --help (or -h)	Print this help\n"
		"    or " <<cmdName << " --version		Print version information\n"
		"\n"
		"Solver options:\n"
        "  -f or --fast		Use fast (but very picky) alternate parser for .pomdp files.\n"
	"  -p or --precision targetPrecision\n"    
"			Set targetPrecision as the target precision in solution \n"
"			quality; run ends when target precision is reached. The target\n" 
"			precision is 1e-3 by default.\n"
	"  --randomization	Turn on randomization for the sampling algorithm.\n"
"			Randomization is off by default.\n"
	"  --timeout timeLimit	Use timeLimit as the timeout in seconds.  If running time\n" 
"			exceeds the specified value, the solver writes out a policy and\n" 
"			terminates. There is no time limit by default.\n"
	"  --memory memoryLimit	Use memoryLimit as the memory limit in MB. No memory limit\n" 
"			by default.  If memory usage exceeds the specified value,\n" 
"			ofsol writes out a policy and terminates. Set the value to be\n" 
"			less than physical memory to avoid swapping.\n"
	"  --trial-improvement-factor improvementConstant\n"
"			Use improvementConstant as the trial improvement factor in the\n"
"			sampling algorithm. At the default of 0.5, a trial terminates at\n" 
"			a belief when the gap between its upper and lower bound is 0.5 of\n" 
"			the current precision at the initial belief.\n" 
		"\n"
		"Policy output options:\n"
       "  -o or --output policyFileName\n"        
"			Use policyFileName as the name of policy output file. The\n" 
"			file name is 'out.policy' by default.\n"
	"  --policy-interval timeInterval\n"       
"			Use timeInterval as the time interval between two consecutive\n" 
"			write-out of policy files. If this is not specified, the solver\n" 
"			only writes out a policy file upon termination.\n"
		"\n"
		"Examples:\n"
		"  " << cmdName << " Hallway.pomdp\n"
		"  " << cmdName << " --timeout 100 --output hallway.policy Hallway.pomdp\n"
		"\n"
		;

//		{"trial_improvement_factor",     1,NULL, 'j'}, // Use ARG as the trial improvement factor. The default is 0.5. So, for example, a trial terminates at a node when its upper and lower bound gap is less than 0.5 of the gap at the root.  


/*	cerr <<
		"usage: " << cmdName << " OPTIONS <model>\n"
		"  -h or --help             Print this help\n"
		"  --version                Print version information\n"
		"\n"
		"Solver options:\n"
		"  -f or --fast             Use fast (but very picky) alternate POMDP parser\n"
		"  -p or --precision        Set target precision in solution quality; run ends when\n"
		"                           target is reached [default: 1e-3]\n"
		"  --randomization          Turn Randomization on for sampling\n"
		"\n"
		"Policy output options:\n"
		"  -o or --output           Specifies name of policy output file [default: 'out.policy']\n"
		"  --timeout                Specifies a timeout in seconds.  If running time exceeds\n"
		"                           the specified value, ofsol writes out a policy\n"
		"                           and terminates [default: no maximum]\n"
		"  --memory                 Specifies the maximum memory usage limit in mege bytes.  If memory usage exceeds\n"
		"                           the specified value, ofsol writes out a policy\n"
		"                           and terminates [default: no maximum]\n"
		"  --policy-interval        Specifies the time interval between two consecutive write-\n"
		"                           out of policy files\n"
		"\n"
		"Examples:\n"
		"  " << cmdName << " Hallway.pomdp\n"
		"  " << cmdName << " --timeout 100 --output hallway.policy Hallway.pomdp\n"
		"\n"
		;*/
	exit(-1);
}


int QMDPSolution(SharedPointer<MOMDP> problem, SolverParams* p)
{
	cout << "Generate QMDP Policy" << endl;
	double targetPrecision = MDP_RESIDUAL;
	// no need to invoke POMDP solver
	// solve MDP
	FullObsUBInitializer m;
	if(problem->XStates->size() != 1 && problem->hasPOMDPMatrices())
	{
		DEBUG_LOG(cout << "Calling FullObsUBInitialize::QMDPSolution_unfac()" << endl;);
		// un-factored 
		// only does this if convert fast is called to produce pomdp version of the matrices
		// need pomdp matrix
		m.QMDPSolution_unfac(problem, targetPrecision); // SYL030909 prevly: m.QValueIteration_unfac(problem, targetPrecision);
		int numActions  = problem->actions->size();
		int numXstates = problem->XStates->size();
		int numYstates = problem->YStates->size();
		m.actionAlphaByState.resize(numActions);
		FOR(a, numActions)
		{
			m.actionAlphaByState[a].resize(numXstates);
			FOR (state_idx, numXstates) 
			{
				m.actionAlphaByState[a][state_idx].resize(problem->getBeliefSize());
			}

		}

		FOR(a, numActions)
		{
			m.UnfacPostProcessing(m.actionAlphas[a], m.actionAlphaByState[a]);
		}
	}
	else
	{
		DEBUG_LOG(cout << "Calling FullObsUBInitialize::QMDPSolution()" << endl;);
		// factored
		m.QMDPSolution(problem, targetPrecision); // SYL030909 prevly: m.QValueIteration(problem, targetPrecision);
		FOR(a, problem->actions->size())
		{
			m.FacPostProcessing(m.actionAlphaByState[a]);
		}
	}

	AlphaPlanePoolSet alphaPlanePoolSet(NULL);
	alphaPlanePoolSet.setProblem(problem);
	alphaPlanePoolSet.setSolver(NULL);
	alphaPlanePoolSet.initialize();
	//addAlphaPlane(alphaPlane);
	
	FOR(a, problem->actions->size())
	{
		for(int stateidx = 0; stateidx < alphaPlanePoolSet.set.size() ; stateidx ++)
		{
			SharedPointer<AlphaPlane> plane (new AlphaPlane());
			copy(*plane->alpha, m.actionAlphaByState[a][stateidx]);
			plane->action = a;
			plane->sval = stateidx;

			alphaPlanePoolSet.set[stateidx]->addAlphaPlane(plane);
		}
	}
	string outFileName (p->outPolicyFileName);
	alphaPlanePoolSet.writeToFile(outFileName, p->problemName);
	return 0;	
}

int FIBSolution(SharedPointer<MOMDP> problem, SolverParams* p)
{
	cout << "Generate FIB Policy" << endl;
	double targetPrecision = MDP_RESIDUAL;
	// no need to invoke POMDP solver

	FastInfUBInitializer f(problem);
	DEBUG_LOG(cout << "Calling FastInfUBInitializer::getFIBsolution()" << endl;);		f.getFIBsolution(targetPrecision);

	AlphaPlanePoolSet alphaPlanePoolSet(NULL);
	alphaPlanePoolSet.setProblem(problem);
	alphaPlanePoolSet.setSolver(NULL);
	alphaPlanePoolSet.initialize();
	//addAlphaPlane(alphaPlane);
	
	FOR(a, problem->actions->size())
	{
		for(int stateidx = 0; stateidx < alphaPlanePoolSet.set.size() ; stateidx ++)
		{
			SharedPointer<AlphaPlane> plane (new AlphaPlane());
			copy(*plane->alpha, f.actionAlphaByState[a][stateidx]);
			plane->action = a;
			plane->sval = stateidx;

			alphaPlanePoolSet.set[stateidx]->addAlphaPlane(plane);
		}
	}
	string outFileName (p->outPolicyFileName);
	alphaPlanePoolSet.writeToFile(outFileName, p->problemName); 
	return 0;	
}

int MDPSolution(SharedPointer<MOMDP> problem, SolverParams* p)
{
    cout << "Generate MDP Policy" << endl;
    double targetPrecision = MDP_RESIDUAL;
    // no need to invoke POMDP solver
    // solve MDP
    FullObsUBInitializer m;
    if(problem->XStates->size() != 1 && problem->hasPOMDPMatrices())
    {
	// un-factored 
	// only does this if convert fast is called to produce pomdp version of the matrices
	// need pomdp matrix
	m.alphaByState.resize(problem->XStates->size());
	DEBUG_LOG(cout << "Calling FullObsUBInitialize::valueIteration_unfac()" << endl;);
	m.valueIteration_unfac(problem, targetPrecision);
	m.UnfacPostProcessing(m.alpha, m.alphaByState);
    }
    else
    {
	// factored
	DEBUG_LOG(cout << "Calling FullObsUBInitialize::valueIteration()" << endl;);
	m.valueIteration(problem, targetPrecision);
	m.FacPostProcessing(m.alphaByState);
    }

    AlphaPlanePoolSet alphaPlanePoolSet(NULL);
    alphaPlanePoolSet.setProblem(problem);
    alphaPlanePoolSet.setSolver(NULL);
    alphaPlanePoolSet.initialize();
    //addAlphaPlane(alphaPlane);

    
    //do one step lookahead if problem is pure MDP
    if(problem->YStates->size() == 1)
    {
	for(int stateidx = 0; stateidx < alphaPlanePoolSet.set.size() ; stateidx ++)
	{
	    SharedPointer<AlphaPlane> plane (new AlphaPlane());
	    int maxAction = 0;
	    double maxActionLB = -DBL_MAX;

	    //search for the best action for this state
	    SharedPointer<BeliefWithState> b = SharedPointer<BeliefWithState>(new BeliefWithState); 
	    b->bvec = new SparseVector(); b->bvec->resize(1);
	    b->bvec->push_back(0,1.0); b->sval=stateidx;
	    //initialise the MDP belief to current state
	    obsState_prob_vector spv;  // outcome probability for values of observed state
	    for(Actions::iterator aIter = problem->actions->begin(); aIter != problem->actions->end(); aIter ++) 
	    {
		int a = aIter.index();

		double sum = 0.0;
		double immediateReward = problem->rewards->getReward(*b, a);
		problem->getObsStateProbVector(spv, *b, a);

		FOR(Xn, spv.size()) 
		{
		    double sprob = spv(Xn);
		    if (sprob > OBS_IS_ZERO_EPS) 
		    {
			double childLB =  m.alphaByState[Xn](0);
			sum += childLB * sprob;
		    }
		}
		sum *= problem->getDiscount();
		sum += immediateReward;

		if(sum > maxActionLB)
		{
		    maxActionLB = sum;
		    maxAction = a;
		}
		assert(maxActionLB !=  -DBL_MAX);
	    }

	    copy(*plane->alpha, m.alphaByState[stateidx]);
	    plane->action = maxAction;
	    plane->sval = stateidx;

	    alphaPlanePoolSet.set[stateidx]->addAlphaPlane(plane);
	}
    }
    else{
	for(int stateidx = 0; stateidx < alphaPlanePoolSet.set.size() ; stateidx ++)
	{
		SharedPointer<AlphaPlane> plane (new AlphaPlane());
		copy(*plane->alpha, m.alphaByState[stateidx]);
		plane->action = -1;
		plane->sval = stateidx;

		alphaPlanePoolSet.set[stateidx]->addAlphaPlane(plane);
	}
    }

    string outFileName (p->outPolicyFileName);
    alphaPlanePoolSet.writeToFile(outFileName, p->problemName);
    return 0;	
}
int main(int argc, char **argv) 
{

	//try
	{
		SolverParams* p = &GlobalResource::getInstance()->solverParams;

		bool parseCorrect = SolverParams::parseCommandLineOption(argc, argv, *p);
		if(!parseCorrect)
		{
			usage(p->cmdName);
			exit(EXIT_FAILURE);
		}


		OutputParams op;
		if(GlobalResource::getInstance()->benchmarkMode)
		{
			if(GlobalResource::getInstance()->simNum == 0|| GlobalResource::getInstance()->simLen == 0)
			{
				cout << "Benchmark Length and/or Number not set, please set them using option --simLen and --simNum" << endl;
				exit(-1);
			}
		}


		GlobalResource::getInstance()->init();
		string baseName = GlobalResource::getInstance()->parseBaseNameWithoutPath(p->problemName);
		GlobalResource::getInstance()->setBaseName(baseName);

		//*************************
		//TODO: parse the problem
		//	long int clk_tck = sysconf(_SC_CLK_TCK);
		//	struct tms now1, now2;
		//	float utime, stime;

#ifdef _MSC_VER
		registerCtrlHanler();
#else
		setSignalHandler(SIGINT, &sigIntHandler);
#endif

		printf("\nLoading the model ...\n  ");

		//Parser* parser = new Parser();  

		GlobalResource::getInstance()->PBSolverPrePOMDPLoad();
		SharedPointer<MOMDP> problem (NULL);
		if(p->hardcodedProblem.length() ==0 )
		{
			problem = ParserSelector::loadProblem(p->problemName, *p);
		}
		else
		{
            cout << "Unknown hard coded problem type : " << p->hardcodedProblem << endl;
            exit(0);
		}

		double pomdpLoadTime = GlobalResource::getInstance()->PBSolverPostPOMDPLoad();
		printf("  loading time : %.2fs \n", pomdpLoadTime);
		GlobalResource::getInstance()->problem = problem;

		//Getting a MDP solutions
		if(p->MDPSolution == true)
		{
			MDPSolution(problem, p);
			return 0;
		}

		if(p->QMDPSolution == true)
		{
			QMDPSolution(problem, p);
			return 0;
		}

		if(p->FIBSolution == true)
		{
			FIBSolution(problem, p);
			return 0;
		}

		if(GlobalResource::getInstance()->benchmarkMode)
		{
			srand(GlobalResource::getInstance()->randSeed);
			GlobalResource::getInstance()->expRewardRecord.resize(GlobalResource::getInstance()->simNum);
		}
		//decide which solver to create
		PointBasedAlgorithm* solver;

		switch (p->strategy)
		{
		case S_SARSOP:
			{
				SARSOP* sarsopSolver = NULL;
				BackupAlphaPlaneMOMDP* lbBackup = new BackupAlphaPlaneMOMDP();
				BackupBeliefValuePairMOMDP* ubBackup = new BackupBeliefValuePairMOMDP();

				sarsopSolver = new SARSOP(problem, p);

				lbBackup->problem = problem;
				sarsopSolver->lowerBoundBackup = lbBackup;

				((BackupAlphaPlaneMOMDP* )(sarsopSolver->lowerBoundBackup))->solver = sarsopSolver;

				ubBackup->problem = problem;
				sarsopSolver->upperBoundBackup = ubBackup;
				solver = sarsopSolver;
			}
			break;

			//case S_FSVI:
			//	solver = new FSVI(problem, p);
			//	break;

			//case S_GES:
			//	if(GlobalResource::getInstance()->migsPathFile != NULL)
			//	{
			//		if(GlobalResource::getInstance()->migsPathFileNum < 0 )
			//		{
			//			GlobalResource::getInstance()->migsPathFileNum = 10;
			//		}
			//		solver = new GES(problem, p, true);
			//	}
			//	else
			//	{
			//		solver = new GES(problem, p);
			//	}
			//	break;

		default:
			assert(0);// should never reach this point
		};

		//solve the problem
		solver->solve(problem);

		cout << endl;

	}

	// Commented out during merge 02102009
	/*catch(bad_alloc &e)
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
	}*/


	return 0;



}


/***************************************************************************
* REVISION HISTORY:
*
***************************************************************************/


//
//
//
//#include <string>
//using namespace std;
//
//
//#include "Belief.h"
//#include "SARSOP.h"
//#include "MOMDP.h"
//#include "MOMDPLite.h"
//#include "PointBasedAlgorithm.h"
//#include "BackupBeliefValuePairMOMDP.h"
//#include "BackupAlphaPlaneMOMDP.h"
//
//
//int main(int argc, char** argv)
//{
//	SARSOP *solver = new SARSOP();
//	string problemName = "something.pomdpx";
//
//	// This section should be the only difference between MOMDP and MOMDP Lite version
//	SharedPointer<MOMDP> problem = MOMDP::LoadProbem(problemName);
//	solver->lowerBoundBackup = new BackupAlphaPlaneMOMDP();
//	solver->upperBoundBackup = new BackupBeliefValuePairMOMDP();
//
//	// MOMDPLite Version
//	//SharedPointer<MOMDP> problem = MOMDPLite::LoadProbem();
//	//solver->lowerBoundBackup = new BackupAlphaPlaneMOMDPLite();
//	//solver->upperBoundBackup = new BackupBeliefValuePairMOMDPLite();
//
//	solver->solve(problem);
//
//
//}
