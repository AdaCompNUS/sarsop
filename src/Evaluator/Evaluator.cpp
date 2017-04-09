#include "MOMDP.h"
#include "ParserSelector.h"
#include "AlphaVectorPolicy.h"
#include "SimulationRewardCollector.h"
#include "BeliefForest.h"
#include "Sample.h"
#include "BeliefCache.h"
#include "EvaluationEngine.h"
#include "SimulationEngine.h"
#include "EvaluatorSampleEngine.h"
#include "EvaluatorBeliefTreeNodeTuple.h"

#include "GlobalResource.h"

#include <string>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <ctime>

#include "CPTimer.h"

#ifdef _MSC_VER
#else
//for timing
#include <sys/param.h>
#include <sys/types.h>
#include <sys/times.h>
//end for timing
#endif

using namespace std;
using namespace momdp;


void print_usage(const char* cmdName) 
{
	cout << "Usage: " << cmdName << " POMDPModelFileName --policy-file policyFileName --simLen numberSteps \n" 
<<"	--simNum numberSimulations [--fast] [--srand randomSeed] [--memory memoryLimit]\n" 
<<"	[--output-file outputFileName]\n" 
<<"    or " << cmdName << " --help (or -h)  Print this help\n" 
<<"    or " << cmdName << " --version	   Print version information\n" 
<<"\n"
<<"Evaluator options:\n"
<<"  --policy-file policyFileName	Use policyFileName as the policy file name (compulsory).\n"
<<"  --simLen numberSteps		Use numberSteps as the number of steps for each\n" 
<<"				simulation run (compulsory).\n"
<<"  --simNum numberSimulations	Use numberSimulations as the number of simulation runs\n" 
<<"				(compulsory).\n"
<<"  -f or --fast			Use fast (but very picky) alternate parser for .pomdp files.\n"
<<"  --srand randomSeed		Set randomSeed as the random seed for simulation.\n" 
<<"				It is the current time by default.\n"
<<"  --memory memoryLimit		Use memoryLimit as the memory limit in MB. No memory\n" 
<<"				limit by default. If memory usage exceeds the specified\n" 
<<"				value, the evaluator will switch back to a more memory\n" 
<<"				conservative (and slow) method.\n"
//<<"  --lookahead yes/no		Set 'yes' ('no') to select action with (without) one-step\n" 
//<<"				look ahead. Action selection is with one-step look ahead\n" 
//<<"				by default.\n" 
<<"\n"
<<"Output options:\n"
<<"  --output-file outputFileName	Use outputFileName as the name for the output file\n" 
<<"				that contains the evaluation trace.\n"
		<< "Example:\n"
		<< "  " << cmdName << " --simLen 100 --simNum 100 --policy-file out.policy Hallway.pomdp\n";

// 	cout << "Usage: binary [options] problem:\n"
// 		<< "--help, print this message\n"
// 		<< "--policy-file, policy file to be used\n"
// 		<< "--output-file, output file to be used\n"
// 		<< "--simLen, length of simulation\n"
// 		<< "--simNum, number of simulations\n"
// 		<< "--srand, random seed (default: current time)\n"
// 		<< "--policy-graph, generate policy graph in DOT format\n"
// 		<< "--graph-depth, specifies the maximum depth of the generated policy graph (default: no limit)\n"
// 		<< "--memory, specifies memory limit in megabytes (default: no limit)\n" 
// 		<< "--lookahead, use \"one-step look ahead\" when selecting action (default: yes)\n"
// 		<< "Examples:\n"
// 		<< " ./evaluate --simLen 100 --simNum 100 --policy-file out.policy Hallway.pomdp\n";

}


void generateSimLog(SolverParams& p, double& globalExpRew, double& confInterval)
{
     int length;
     char str1[102];
     string str_comb;

     int startpos = 0;
     int i;
     for (i = p.problemName.length() - 1; i >= 0; i--) {
	  if (p.problemName[i] == '/') {
	       startpos = i + 1;
	       break;
	  }
     }

     str_comb.append(p.problemName.begin() + startpos, p.problemName.end());

     str_comb.append("SimLog");
     cout << str_comb << endl;

     length = str_comb.copy(str1, 100);
     str1[length] = '\0';

     FILE *fp = fopen(str1, "a");

     //  FILE *fp = fopen("sim.log","a");
     if (fp == NULL) 
     {
	  cerr << "cant open sim logfile\n";
	  exit(1);
     }

     fprintf(fp, "%f ", globalExpRew);
     fprintf(fp, "%f ", globalExpRew - confInterval);
     fprintf(fp, "%f ", globalExpRew + confInterval);
     fprintf(fp, "\n");
     fclose(fp);


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
	  if (p->policyFile == "" || p->simLen == -1 || p->simNum == -1) 
	  {
	       print_usage(p->cmdName);
	       return 0;
	  }


	  bool enableFiling = false;

	  if (p->outputFile.length() == 0) 
	  {
	       enableFiling = false;
	  } 
	  else 
	  {
	       enableFiling = true;
	  }

	  cout << "\nLoading the model ..." << endl << "  ";
	  SharedPointer<MOMDP> problem = ParserSelector::loadProblem(p->problemName, *p);

	  if (problem->initialBeliefStval->sval == -1) 
	  { 
	       cerr << "\nPlease use the simulator. Random initial value for the fully observable state variable is not supported in the evaluator.\n";
	       exit(1);
	   }

	  cout << "\nLoading the policy ..." << endl;
	  cout << "  input file   : " << p->policyFile << endl;
	  SharedPointer<AlphaVectorPolicy> policy = new AlphaVectorPolicy(problem);
	  bool policyRead = policy->readFromFile(p->policyFile);
	  if(!policyRead)
	  {
	       return 0;
	  }

	  cout << "\nSimulating ..." << endl;
	  if(p->useLookahead)
	  {
	    cout << "  action selection :  one-step look ahead" << endl;
	  }
	  else
	  {
	  }

	  SimulationRewardCollector rewardCollector;
	  rewardCollector.setup(*p);


	  vector <BeliefCache *> beliefCacheSet;
	  int xStateNum = problem->XStates->size();
	  beliefCacheSet.resize(xStateNum);

	  for(States::iterator iter = problem->XStates->begin(); iter != problem->XStates->end(); iter ++ )
	  {
	       beliefCacheSet[iter.index()] = new BeliefCache();
	  }

	  BeliefForest* beliefForest = new BeliefForest();
	  EvaluatorSampleEngine* sample = new EvaluatorSampleEngine();

	  sample->setup(NULL, problem, &beliefCacheSet, beliefForest);
	  beliefForest->setup(problem, sample, &beliefCacheSet);
	  beliefForest->globalRootPrepare();


	  ofstream * foutStream = NULL;
	  srand(p->seed);//Seed for random number.  Xan
	  //cout << p->seed << endl;

	  //MOVED SYLTAG
	  // if the bvec field of problem.initialBeliefStval is not specified
	  SharedPointer<BeliefWithState> startBeliefStval(new BeliefWithState());

	  copy(*startBeliefStval->bvec, *problem->initialBeliefStval->bvec);
	  startBeliefStval->sval = problem->initialBeliefStval->sval;

	  belief_vector startBel;
	  copy(startBel, *startBeliefStval->bvec);

	  //belief_vector startBel = problem.initialBelief;
	  if (startBel.filled() == 0) 
	  {
	       throw runtime_error("startBel.filled() == 0 !?");
	       int numStates = problem->getBeliefSize();
	       startBel.resize(numStates);
	       for (int i = 0; i < numStates; i++) 
	       {
		    startBel.push_back(i, ((double) 1) / (double(numStates)));
	       }
	       copy(*startBeliefStval->bvec, startBel);
	  }

	  //ADD SYLTAG
	  belief_vector startBeliefX;
	  // check if startBeliefStval->sval is specified or is it random start value for X
	  if (startBeliefStval->sval == -1) 
	  { 
	       // random start value for X
	       copy(startBeliefX, *problem->initialBeliefX);
	  } 
	  else 
	  { // for completeness we have a valid startBeliefX
	       startBeliefX.resize(problem->XStates->size());
	       startBeliefX.push_back(startBeliefStval->sval, 1.0);
	  }

	  //CPTimer simTimer;

	  bool hasMemory = true;
	  if (enableFiling) 
	  {
	       foutStream = new ofstream(p->outputFile.c_str());
	  }

	  for (int currSim = 0; currSim < p->simNum; currSim++) 
	  {
	       double reward = 0, expReward = 0;

	       if(hasMemory)
	       {
		    try
		    {
			 EvaluationEngine engine;
			 engine.setup(problem, policy, beliefForest, &beliefCacheSet, sample, p);
			 int firstAction = engine.runFor(p->simLen, *startBeliefStval, startBeliefX, foutStream, reward, expReward);
			 if(firstAction < 0)
			 {
			      // something wrong happend, exit
			      return 0;
			 }
		    }
		    catch(exception &e)
		    {
			 cout << "Memory limit reached, switch from evaluation to simulation and continue..." << endl;
			 hasMemory = false;
			 // TODO:: should free memory..., but for now, let's just remove the memory limit and continue
			 GlobalResource::getInstance()->solverParams.memoryLimit = 0;
			 delete beliefForest;
		    }
	       }

	       if(!hasMemory)
	       {
		    SimulationEngine engine;
		    engine.setup(problem, policy, p);
		    int firstAction = engine.runFor(p->simLen, foutStream, reward, expReward);

		    if(firstAction < 0)
		    {
			 // something wrong happend, exit
			 return 0;
		    }
	       }

	       rewardCollector.addEntry(currSim, reward, expReward);
	       rewardCollector.printReward(currSim);


	  }

	  if (enableFiling)
	  {
	       foutStream->close();
	  }


	  rewardCollector.printFinalReward();
	  DEBUG_LOG( generateSimLog(*p, rewardCollector.globalExpRew, rewardCollector.confInterval); );
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

