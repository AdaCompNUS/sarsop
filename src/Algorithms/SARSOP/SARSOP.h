#ifndef SARSOP_H
#define SARSOP_H
#include <list>
#include "PointBasedAlgorithm.h"
#include "Bound.h"
#include "BoundSet.h"
#include "AlphaPlane.h"
#include "BeliefCache.h"
#include "BeliefForest.h"
#include "BinManager.h"
#include "BinManagerSet.h"
#include "SampleBP.h"
#include "AlphaPlanePool.h"
#include "AlphaPlanePoolSet.h"
#include "BeliefValuePair.h"
#include "BeliefValuePairPool.h"
#include "BeliefValuePairPoolSet.h"
#include "AlphaPlaneMaxMeta.h"
#include "Tuple.h"
#include "FacmodelStructs.h"
#include "GlobalResource.h"


using namespace std;
namespace momdp 
{
	class BinManager;
	class SARSOPPrune;

#define CHECK_INTERVAL 50


	class SARSOPAlphaPlaneTuple :public Tuple
	{
	public:
		int certed;//for recording the usage as max plane at some belief points 
		vector<BeliefTreeNode*> certifiedBeliefs;	// only non-fringe nodes
		vector<int> certifiedBeliefTimeStamps;
		vector<AlphaPlaneMaxMeta*> maxMeta; // all nodes
		bool sanityMax;

	};

	class SARSOP :	public PointBasedAlgorithm
	{
	private:


	public:

		SARSOP(SharedPointer<MOMDP> problem, SolverParams * solverParams);
		virtual ~SARSOP(void);

		BeliefValuePairPoolSet *upperBoundSet;
		AlphaPlanePoolSet *lowerBoundSet;

		BinManagerSet* binManagerSet;

		SARSOPPrune* pruneEngine;

		Backup<BeliefValuePair> *upperBoundBackup;
		Backup<AlphaPlane> *lowerBoundBackup;

		vector<IndexedTuple<AlphaPlanePoolDataTuple> *> lbDataTableSet;
		vector<IndexedTuple<BeliefValuePairPoolDataTuple> *> ubDataTableSet;



		virtual void solve(SharedPointer<MOMDP> problem);
		virtual void writePolicy(string fileName, string problemName);
		void writeToFile(const std::string& outFileName, string problemName);

		BeliefTreeNode* sample();
		void backup(BeliefTreeNode* node);

		// Callback functions, must be static, or else it will not match the required signature due the member function's implicit "this" pointer
		static void onLowerBoundBackup (PointBasedAlgorithm *solver, BeliefTreeNode * node, SharedPointer<AlphaPlane> backupResult)
		{
			// updating certs, etc
		}

		static void onUpperBoundBackup (PointBasedAlgorithm *solver, BeliefTreeNode * node, SharedPointer<BeliefValuePair> backupResult)
		{
		}

		static void onGetNode(PointBasedAlgorithm *solver, BeliefTreeNode* node, SharedPointer<BeliefWithState>& belief)
		{
			SARSOP *sarsopSolver = (SARSOP *)solver;
			int stateidx = belief->sval;
			int row = node->cacheIndex.row;
			int timeStamp = sarsopSolver->numBackups;

			// SARSOP Bin Manager related
			sarsopSolver->binManagerSet->binManagerSet[stateidx]->binManagerDataTable.set(row).binned = false;

			// TODO: fix this bug, UB_ACTION is set by backup, but if a node is allocated and sampled before backup, UB_ACTION is not defined
			sarsopSolver->upperBoundSet->set[stateidx]->dataTable->set(row).UB_ACTION = 0;
			

			list<SharedPointer<AlphaPlane> >* alphas = new list<SharedPointer<AlphaPlane> >();
			sarsopSolver->lowerBoundSet->set[stateidx]->dataTable->set(row).ALPHA_PLANES= alphas;

			// TODO:: fix it
			SharedPointer<AlphaPlane>alpha = sarsopSolver->lowerBoundSet->getValueAlpha(belief);
			//REAL_VALUE lbVal = sarsopSolver->lowerBoundSet->getValue(belief);
			REAL_VALUE lbVal = inner_prod(*alpha->alpha, *belief->bvec);
			//REAL_VALUE lbVal = bounds->getLowerBoundValue(b_s, &alpha);



			SARSOPAlphaPlaneTuple *dataAttachedToAlpha = (SARSOPAlphaPlaneTuple *)(alpha->solverData);
			//TODO: dataAttachedToAlpha->maxMeta



			REAL_VALUE ubVal =sarsopSolver->upperBoundSet->getValue(belief);
			// TODO:: node->lastUBVal = ubVal;

			solver->beliefCacheSet[stateidx]->getRow( row)->REACHABLE = node;
			solver->beliefCacheSet[stateidx]->getRow( row)->UB = ubVal;
			solver->beliefCacheSet[stateidx]->getRow( row)->LB = lbVal;

			sarsopSolver->lowerBoundSet->set[stateidx]->dataTable->set(row).ALPHA_TIME_STAMP = timeStamp;


			if(timeStamp!=-1)
			{
				//assert(solver->beliefCacheSet[stateidx]->getRow( row)->isFringe );
				DEBUG_TRACE("getNode timeStamp!=-1");
				if(!hasMaxMetaAt(alpha, node->cacheIndex.row))	// assume that the alpha is from the correct boundsSet[]
				{
					DEBUG_TRACE("!hasMaxMetaAt");
					AlphaPlaneMaxMeta* newMax = new AlphaPlaneMaxMeta();
					newMax->cacheIndex = node->cacheIndex.row;
					newMax->lastLB = lbVal;
					newMax->timestamp = GlobalResource::getInstance()->getTimeStamp();
					dataAttachedToAlpha->maxMeta.push_back(newMax);
				}
			}


		}

		// Initialization

		void initialize(SharedPointer<MOMDP> problem);
		void initSampleEngine(SharedPointer<MOMDP> problem);
		void initializeUpperBound(SharedPointer<MOMDP> problem);
		void initializeLowerBound(SharedPointer<MOMDP> problem);
		void initializeBounds(double _targetPrecision);

		// Alpha Vector Related
		static bool hasMaxMetaAt(SharedPointer<AlphaPlane>alpha, int index)
		{
			SARSOPAlphaPlaneTuple *attachedData = (SARSOPAlphaPlaneTuple *)alpha->solverData;
			FOREACH(AlphaPlaneMaxMeta* , entry, attachedData->maxMeta)
			{
				if((*entry)->cacheIndex == index)
				{
					return true;
				}
			}
			return false;
		}

		// Timers
		CPTimer	runtimeTimer;
		CPTimer lapTimer;

		double elapsed;

		// Print
		int printIndex;

		void alwaysPrint();
		void printHeader();
		void printDivider();
		void print();
		bool stopNow();

		BeliefTreeNode& getMaxExcessUncRoot(BeliefForest& globalroot); //ADD SYLTAG

		void writeIntermediatePolicyTraceToFile(int trial, double time, const string& outFileName, string problemName);
		void progressiveIncreasePolicyInteval(int& numPolicies);
		void logFilePrint(int index);		//SYL ADDED FOR EXPTS

		// backup methods
		cacherow_stval backup(list<cacherow_stval> beliefNStates); //modified for factored, prevly: int backup(list<int> beliefs);
		cacherow_stval backupLBonly(list<cacherow_stval> beliefNStates); //modified for factored, prevly: int backup(list<int> beliefs);

		//second level methods
		cacherow_stval backup(cacherow_stval beliefNState); //modified for factored, prevly: int backup(int belief);
		cacherow_stval backupLBonly(cacherow_stval beliefNState); //modified for factored, prevly: int backup(int belief);

	};

}

#endif
