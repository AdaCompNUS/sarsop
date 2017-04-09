/** 
* Part of the this code is derived from ZMDP: http://www.cs.cmu.edu/~trey/zmdp/
* ZMDP is released under Apache License 2.0
* The rest of the code is released under GPL v2 
*/

#include <assert.h>
#include <stdlib.h>
#include <sstream>
#include "GlobalResource.h"

#ifdef _MSC_VER
#else
#include <unistd.h>
#endif

#if defined(_MSC_VER) || defined(_CYGWIN)
#include "getopt.h"
#else
#include <getopt.h>
#endif

#ifdef __cplusplus
extern "C"
#endif
{
	extern unsigned long GlobalMemLimit;
}



#include <stdio.h>
#include <ctime>

#include <iostream>
#include <fstream>
#include <string.h>
#include <algorithm>

#include "solverUtils.h"

using namespace std;
using namespace momdp;

namespace momdp{

	/**********************************************************************
	* HELPER FUNCTIONS
	**********************************************************************/

	static EnumEntry strategiesG[] = {
		{"sarsop", S_SARSOP},
		{"sarsop_bp", S_BPS},
		{NULL, -1}
	};

	int getEnum(const char* key, EnumEntry* table, const char* cmdName, const char *opt)
	{
		EnumEntry* i = table;
		for (; NULL != i->key; i++) {
			if (0 == strcmp(i->key, key)) {
				return i->val;
			}
		}
		fprintf(stderr, "ERROR: invalid parameter %s for option %s\n\n", key, opt);
		//usage(cmdName);
		exit(EXIT_FAILURE);
	}

	bool endsWith(const std::string& s,
		const std::string& suffix)
	{
		if (s.size() < suffix.size()) return false;
		return (s.substr(s.size() - suffix.size()) == suffix);
	}

	/**********************************************************************
	* EXPORTED API
	**********************************************************************/

	SolverParams::SolverParams(void)
	{
		MDPSolution = false;
		QMDPSolution = false;
		FIBSolution = false;
		useLookahead = true;
		graphDepth = 0; // no limit
		graphMaxBranch = 0; // no limit
		graphProbThreshold = 0;
		simLen = -1;
		simNum = -1;
		seed = time(0);
		//stateMapFile = ""; // Commented out during code merge on 02102009
		
		memoryLimit = 0; // no memory limit
		strategy = S_SARSOP;
		//probName = NULL; // Commented out during code merge on 02102009
		targetPrecision = 1e-3;
		useFastParser = false;
		doConvertPOMDP = false;
		outPolicyFileName = "";
		interval = -1;
		timeoutSeconds = -1;
		delta = 0.1;
		randomizationBP = false;
		overPruneThreshold = 50.0;
		lowerPruneThreshold = 5.0;
		dynamicDeltaPercentageMode = false;
		BP_IMPROVEMENT_CONSTANT = 0.5;

		targetTrials = 0;
		dumpPolicyTrace = false;
		dumpPolicyTraceTime = false;

		outPolicyFileName = "out.policy"; // default value
		dumpData = false;



	}

	void SolverParams::setStrategy(const char* strategyName)
	{
		strategy = getEnum(strategyName, strategiesG, cmdName, "--search");
	}

	void SolverParams::inferMissingValues(void)
	{
		// fill in default and inferred values
	}



	bool SolverParams::parseCommandLineOption(int argc, char **argv, SolverParams& p)
	{
		static char shortOptions[] = "hp:t:v:fo:";
		static struct option longOptions[]={

	// ***** common options
		{"help",			0,NULL,'h'}, // display help
		{"version",			0,NULL,'V'}, // print version 
		{"fast",			0,NULL,'f'}, // Use fast (but very picky) alternate parser for .pomdp files.
		{"memory",			1,NULL,'m'}, // Use ARG as the memory limit in MB. No memory limit by default. [for ofsol and evaluate only]

	// ***** ofsol only options
		{"precision",			1,NULL,'p'}, // Set ARG as the target precision in solution quality. The target precision is 1e-3 by default. Note, this precision is also used in initialization 
		{"randomization",		0,NULL,'c'}, // SampleBP randomization flag. Turn on randomization for the sampling algorithm. Randomization is off by default.
		{"timeout",			1,NULL,'T'}, // Use ARG as the timeout in seconds.  There is no time limit by default.
		{"output",			1,NULL,'o'}, //  Use ARG as the name of policy output file. The file name is "out.policy" by default.
		{"policy-interval",		1, NULL, 'i'}, // Use ARG as the time interval between two consecutive write-out of policy files. If this is not specified, ofsol only writes out a policy file upon termination.
		{"trial-improvement-factor",     1,NULL, 'j'}, // Use ARG as the trial improvement factor. The default is 0.5. So, for example, a trial terminates at a node when its upper and lower bound gap is less than 0.5 of the gap at the root.  

		// --------- internal use
		{"unfactored-init",		0, NULL, 'M' }, // see documentation below
		{"hardcoded",			1, NULL, 'H' }, // for using hard coded problem instead of loading problem from command line
		{"mdp",				0, NULL, 'W' }, // for generating MDP policy
		{"qmdp",			0, NULL, 'X' }, // for generating QMDP policy
		{"fib",				0, NULL, 'F' }, // for generating FIB policy
		{"overPruneThreshold",		1,NULL, 'b'}, // Dynamic delta pruning parameter 
		{"lowerPruneThreshold",		1,NULL, 'g'}, // Dynamic delta prunning prarm  
		{"trials",			1,NULL, 'N'}, // target trials, not used
		{"dump",			0,NULL, 'D'}, // flag to dump data, not used
		//{"search",			1,NULL, 's'}, // algorithm selection, no longer used, since we only have SARSOP now 
		{"dumpPolicyTrace",		0,NULL, 'P'}, // dump policy at regular interval, not used

	// ***** simulate, evaluate and policygraph options
		{ "policy-file",		1, NULL, 'Q' }, // Use ARG as the policy file name (compulsory).

		// --------- internal use
		{ "lookahead",			1, NULL, 'L' }, // ARG == no means do not use one step look ahead for action selection. Default uses one step look ahead for action selection. 

	// ***** simulate and evaluate options
		{ "simLen",			1, NULL, 'S' }, // Use ARG as the number of steps for each simulation run (compulsory).
		{ "simNum",			1, NULL, 'U' }, // Use ARG as the number of simulation runs (compulsory).
		{ "srand",			1, NULL, 'R' }, // Set ARG as the random seed for simulation. It is the current time by default.
		{ "output-file",		1, NULL, 'O' }, // Use ARG as the name for the output file that contains the simulation trace.


	// ***** simulate only options
		// --------- internal use
		{ "statemap",			1, NULL, 'A' }, // Simulator generates mapping from unobserved state to variable value in the file named ARG.

	// ***** policygraph only options
		{ "policy-graph",		1, NULL, 'G' }, // Generate a policy graph in DOT format in the file named ARG.
		{ "graph-max-depth",		1, NULL, 'd' }, // Use ARG as the maximum horizon of the generated policy graph. There is no limit by default.
		{ "graph-max-branch",		1, NULL, 'B' }, // ARG is the max policy graph outgoing edges per node. There is no limit by default.
		{ "graph-min-prob",		1, NULL, 't' }, // Suppress policy graph with outgoing edges less than probability ARG. There is no minimum threshold by default.

		{NULL,0,0,0}

		};

		// Command Line flag documentation:
		// ===========================================
		// unfactored-init:
		// -------------------------------------------
		// If input is a .pomdpx file with some fully observed state variables, the
		// default is to NOT create duplicate (flat pomdp) matrices and to do
		// FACTORED initialization for upper and lower bounds.
		// If flag "--unfactored-init" is used, then duplicate (flat pomdp) matrices
		// are created and unfactored initialization for upper and lower bounds is
		// done.
		// The flag works in conjunction with the "--mdp", "--qmdp" and "--fib" flags as well.
		// The MDP, QMDP and FIB policies are based on the upper bound initialization functions.
		// The UNFACTORED version is chosen with the flag "--unfactored-init".
		//
		// The flag does not affect the following cases:
		// Input is a .pomdpx file with no fully observed state variables (like in
		// tiger.pomdpx).
		// Input is a .pomdp file.
		// (There's no concept of factored initialization for these two cases).
		// ==========================================


		p.cmdName = argv[0];

		while (1) 
		{
			char optchar = getopt_long(argc,argv,shortOptions,longOptions,NULL);
			if (optchar == -1) break;

			switch (optchar) 
			{
			case 'A':
				{
					p.stateMapFile = string(optarg);
				}
				break;
			case 'H':
				{
					p.hardcodedProblem = string(optarg);
				}
				break;
			case 'M':
				{
					p.doConvertPOMDP = true;
				}
				break;
			case 'B':
				{
					p.graphMaxBranch = atoi(optarg);
				}
				break;
			case 'G':
				{
					p.policyGraphFile = string(optarg);
				}
				break;
			case 'd':
				{
					p.graphDepth = atoi(optarg);
				}
				break;
			case 't':
				{
					p.graphProbThreshold = atof(optarg);
				}
				break;

			case 'h': // help
				return false;
				break;
			case 'V': // version
				cout << "Approximate POMDP Planning (APPL) Toolkit Version 0.9" << endl;
				exit(EXIT_SUCCESS);
				break;
			case 's': // search
				p.setStrategy(optarg);
				break;
			case 'f': // fast
				p.useFastParser = true;
				break;
			case 'p': // precision
				p.targetPrecision = atof(optarg);
				break;
			case 'b': // over prune threshold
				p.overPruneThreshold = atof(optarg);
				break;
			case 'g': // over prune threshold
				p.lowerPruneThreshold = atof(optarg);
				break;
			case 'j': // bp improvement constant
				p.BP_IMPROVEMENT_CONSTANT = atof(optarg);
				break;
			case 'm': // memory limit
				{
					double limit = atof(optarg);
					p.memoryLimit = (unsigned long)(limit * 1024*1024);
					GlobalMemLimit = (unsigned long)(limit * 1024*1024);
				}
				break;
			case 'l':
				p.dynamicDeltaPercentageMode = true;
				break;
			case 'c': // do randmization in BP
				p.randomizationBP = true;
				break;

			case 'N': // number of trials
				p.targetTrials = atoi(optarg);
				break;

			case 'D': // dump.  Xan 07-08-2007
				p.dumpData = true;
				break;
			case 'P': // dump policy trace, by Yanzhu
				p.dumpPolicyTrace = true;
				break;
			case 'I': // dump policy trace, by Yanzhu
				p.dumpPolicyTraceTime = true;
				break;
			case 'o': // output
				p.outPolicyFileName = string(optarg);
				break;
			case 'T': // timeout
				p.timeoutSeconds = atof(optarg);
				break;
			case 'i':
				p.interval = atof(optarg);
				break;
			case 'a':
				p.delta = atof(optarg);
				break;
			case 'W':
				p.MDPSolution = true;
				break;
			case 'X':
				p.QMDPSolution = true;
				break;
			case 'F':
				p.FIBSolution = true;
				break;


			case 'L':
				{
					string useLookahead(optarg);
					if(useLookahead.compare("yes") == 0)
					{
						p.useLookahead = true;
					}
					else
					{
						p.useLookahead = false;
					}
				}
				break;
			case 'Q':
				{
					p.policyFile = string(optarg);
				}
				break;
			case 'O':
				{
					p.outputFile = string(optarg);
				}
				break;
			case 'S':
				p.simLen = atoi(optarg);
				break;
			case 'U':
				p.simNum = atoi(optarg);
				break;
			case 'R':
				p.seed = atoi(optarg) >= 0 ? atoi(optarg) : time(0);
				break;

			case '?': // unknown option
			case ':': // option with missing parameter
				// getopt() prints an informative error message
				cerr << endl;
				return false;
				break;
			default:
				cerr << "unknowm paramter specified" << endl << endl;
				return false;
			}
		}
		if (argc-optind != 1) 
		{
			if(p.hardcodedProblem.length() > 0 )
			{
				cout << "Using hardcoded problem : " << p.hardcodedProblem << endl;
			}
			else
			{
				cerr << "Error: no arguments were given." << endl << endl;
				return false;
			}
		}

		// p.probName = argv[optind++]; // Commented out during code merge on 02102009
		p.inferMissingValues();

		if( p.hardcodedProblem.length() ==0 )
		{
			p.problemName = string(argv[optind++]);
		
			// check pomdp file name
			std::string probNameStr = p.problemName;
			std::string suffixStr(".pomdp");
			std::string suffixStr2(".pomdpx");
			std::transform(probNameStr.begin(), probNameStr.end(), probNameStr.begin(), ::tolower);

			bool test1 = endsWith(probNameStr, suffixStr);
			test1 |= endsWith(probNameStr, suffixStr2);
			if (test1) 
			{
				// filename looks ok
			} 
			else 
			{
				cerr << "ERROR: only POMDP or POMDPX file format with suffix .pomdp or .pomdpx are supported. The specified file: "<< p.problemName << " is not supported." << endl<< endl;
				return false;
			}

			p.problemBasenameWithoutPath = GlobalResource::parseBaseNameWithoutPath(p.problemName);
			p.problemBasenameWithPath = GlobalResource::parseBaseNameWithPath(p.problemName);
		}
		else
		{
			p.problemName = p.hardcodedProblem;
			p.problemBasenameWithoutPath = p.hardcodedProblem;
			p.problemBasenameWithPath = p.hardcodedProblem;

		}
		return true;
	}
}; // namespace momdp

