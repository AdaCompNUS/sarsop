/** 
* Part of the this code is derived from ZMDP: http://www.cs.cmu.edu/~trey/zmdp/
* ZMDP is released under Apache License 2.0
* The rest of the code is released under GPL v2 
*/

#ifndef INCsolverUtils_h
#define INCsolverUtils_h

#include <iostream>
#include <string>

using namespace std;

// search strategies
//#include "HSVI.h"

// problem types
#include "POMDP.h"
//#include "MDPSim.h"
//#include "PomdpSim.h"


#include "MObject.h"
using namespace momdp;

namespace momdp{

	struct EnumEntry {
		const char* key;
		int val;
	};

	enum StrategiesEnum {
		S_HSVI,
		S_FSVIHSVI,
		S_PERSEUS,
		S_PBVI,
		S_SARSOP,
		S_GES,
		S_FSVI,
		S_COLLECT,
		S_CA,
		S_BP,
		S_BPS
	};

	class SolverParams  : public MObject
	{
	public:

		unsigned long memoryLimit; // memory limit in bytes
		bool useLookahead;
		string policyFile;
		string outputFile;
		string problemName;
		string stateMapFile;
		string problemBasenameWithoutPath;
		string problemBasenameWithPath;
		string policyGraphFile;
		string hardcodedProblem;

		int graphDepth;
		int graphMaxBranch;
		double graphProbThreshold;

		bool MDPSolution;
		bool QMDPSolution;
		bool FIBSolution;

		int simLen;
		int simNum;
		int seed;

		const char* cmdName;
		int strategy;
		int targetTrials;
		//const char *probName;
		double targetPrecision;
		bool useFastParser; // use fast parser to load POMDP ? applicable to RockSample only
		bool doConvertPOMDP; // convert POMDPX to POMDP for initialization, take more memory, but may result in faster initialization
		string outPolicyFileName;
		double interval;
		double timeoutSeconds;
		double delta; // Delta pruning parameter
		double overPruneThreshold;
		double lowerPruneThreshold;
		bool	dynamicDeltaPercentageMode;
		double BP_IMPROVEMENT_CONSTANT;

		bool dumpData;//added Xan 07-08-2007
		bool dumpPolicyTrace; // added by Yanzhu
		bool dumpPolicyTraceTime;
		bool randomizationBP;


		SolverParams(void);
		void setStrategy(const char* strategyName);
		void inferMissingValues(void);

		static bool parseCommandLineOption(int argc, char **argv, SolverParams& p);
	};

	/*
	struct SolverObjects {
	PointBasedAlgorithm* solver;
	Bounds* bounds;
	POMDP* problem;
	//  SimulationEngine* sim;
	};

	void constructSolverObjects(SolverObjects& obj, const SolverParams& p);
	*/
}; // namespace momdp

#endif // INCsolverUtils_h

/***************************************************************************
* REVISION HISTORY:
*
***************************************************************************/
