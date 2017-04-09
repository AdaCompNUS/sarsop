/** 
 * The code is released under GPL v2 
 */
/***************************************************************************
File: SampleBP.h
Data: 20/03/2007
Author: rn
Brief: Sample subclass for BP sampling
 ***************************************************************************/
#ifndef SAMPLEBP_H
#define SAMPLEBP_H

#define BP_INIT_MAX_DEPTH (10)
#define BP_MAX_DEPTH_ADJUST_RATIO (1.1)
//#define BP_IMPROVEMENT_CONSTANT (0.95)  // now is a variable in SolverParams

#define USE_HSVI_ADAPTIVE_DEPTH 1

#include "Sample.h"
#include "BeliefCache.h"
#include "BinManager.h"
#include "BinManagerSet.h"
#include "PointBasedAlgorithm.h"
#include "BeliefTreeNode.h"


using namespace std;
using namespace momdp;

/****************************************
  DECLARATION OF SampleBP
 ****************************************/

namespace momdp{


    class SARSOP;

    struct BPUpdateResult {
	int maxUBAction;
	double maxUBVal;
	double ubResidual;
	int maxExcessUncStateOutcome;  // chosen value of observed state variable	
	int maxExcessUncOutcome;	// chosen value of observation variable
	double maxExcessUnc;
    };

    class SampleBP: public Sample{
	private:
	    //attributes
#if USE_HSVI_ADAPTIVE_DEPTH
	    std::vector<int> depthArr; //int depth;		// SYLMOD17102008 for parallel trials
	    double logOcc;
	    double maxDepth,oldMaxDepth;
	    double oldQualitySum, newQualitySum;
	    int oldNumUpdates, newNumUpdates;
#endif
	    //  std::vector<double> trialTargetPrecisionArr;  // double trialTargetPrecision;		// SYLMOD17102008 for parallel trials



	    //BeliefCache* beliefCache;  // removed for factored, now accessed through Bounds class
	    std::vector<list<cacherow_stval> > priorityQueueArr; //list<cacherow_stval> priorityQueue; // modified for factored, prevly, list <int> priorityQueue;
	    // SYLMOD17102008 for parallel trials

	public:
	    int numTrials;
	    int numSubOptimal; //xan-edit to count no of suboptimal paths found
	    //double targetPrecision; //xan-edit		// SYLMOD17102008 doesnt seem to be used
	    BinManagerSet * bm;//for Bin Heuristic
	    int numBinProceed;//for Bin Heuristic
	    std::vector<double> nextNodeTargetUbArr;  //double nextNodeTargetUb;  // SYLMOD17102008 for parallel trials //for BP (revised)
	    bool isRoot;//for BP (revised)
	    std::vector<double> nextNodeTargetLbArr; //double nextNodeTargetLb;// SYLMOD17102008 for parallel trials //new heuristic

	    std::vector<double> trialTargetPrecisionArr;

	    std::vector<int> newTrialFlagArr; //int newTrialFlag; //ADD SYLTAG : 1 indicates to the chooser of roots that a new trial should start 
	    // SYLMOD17102008 for parallel trials
	    //constructor
	    SampleBP();
	    virtual void setup(SharedPointer<MOMDP> _problem, SARSOP* _solver);

	    SARSOP* solver;
	    //methods
	    list<cacherow_stval> sample(cacherow_stval currentBeliefIndex, unsigned int currentRoot);
	    // SYLMOD17102008 for parallel trials
	    //list<cacherow_stval> sample(cacherow_stval currentBeliefIndex); // prevly, list<int> sample(int currentBeliefIndex);
	    void getMaxExcessUncOutcome(BeliefTreeNode& cn, BPUpdateResult& r, unsigned int currentRoot) const;
	    // SYLMOD17102008 for parallel trials	
	    //void getMaxExcessUncOutcome(BeliefTreeNode& cn, BPUpdateResult& r) const;

	    void setTargetPrecision(double _targetPrecision);
	    void setBinManager(BinManagerSet* _bm);
	    //bool CompareIfLowerBoundImprovesAction(int action, int observation, BeliefTreeNode & currentNode);
	    bool CompareIfLowerBoundImprovesAction(int action, int observation, int xstate, BeliefTreeNode & currentNode, unsigned int currentRoot);
	    // SYLMOD17102008 for parallel trials		
	    //bool CompareIfLowerBoundImprovesAction(int action, int observation, int xstate, BeliefTreeNode & currentNode);
	    double calculateLowerBoundValue(int action, BeliefTreeNode & currentNode);
	    int chooseAction(BeliefTreeNode & currentNode);

	    bool doRandomization;
	    void setRandomization(bool newFlag);
	    ~SampleBP(){}
    };//end class SampleBP

};//end namespace momdp
#endif
