/** 
* Part of the this code is derived from ZBeliefTree: http://www.cs.cmu.edu/~trey/zmdp/
* ZBeliefTree is released under Apache License 2.0
* The rest of the code is released under GPL v2 
*/

/********** tell emacs we use -*- c++ -*- style comments *******************
$Revision: 2.10 $  $Author: duyanzhu $  $Date: 2008/06/14 01:41:14 $

@file    Sample.h
@brief   the superclass of all the sample classes

***************************************************************************/
#ifndef Sample_H
#define Sample_H

#include <list>
#include <vector>
#include "Belief.h"
#include "BeliefTreeNode.h"
#include "BeliefForest.h"
#include "PointBasedAlgorithm.h"
using namespace std;
using namespace momdp;

namespace momdp
{
	class MOMDP;


	class Sample
	{
	public:
		typedef void (*GetNodeCallback)(PointBasedAlgorithm *solver, BeliefTreeNode *node, SharedPointer<BeliefWithState>& belief) ;
		
	protected:
		vector<GetNodeCallback> onGetNode;

		vector <BeliefCache *> *beliefCacheSet;
		BeliefForest *beliefForest;


	public:
		SharedPointer<MOMDP> problem;
		PointBasedAlgorithm* solver;

		int numStatesExpanded;
		bool dumpData;
		bool dumpPolicyTrace;
		bool dumpPolicyTraceTime;

		Sample()
		{
			numStatesExpanded = 0;
		}

		virtual void setup(PointBasedAlgorithm* _solver, SharedPointer<MOMDP> _problem, vector <BeliefCache *> *_beliefCacheSet, BeliefForest* _beliefForest)
		{
			solver = _solver;
			problem = _problem;
			beliefCacheSet = _beliefCacheSet;
			beliefForest = _beliefForest;

		}
		
		virtual void appendOnGetNodeHandler(GetNodeCallback _callback)
		{
			onGetNode.push_back( _callback);
		}

		
		
		virtual list<cacherow_stval> sample(cacherow_stval beliefIndex, unsigned int currentRoot) // modified for factored, prevly, virtual list<int> 
		{
			return list<cacherow_stval>();
		}

		//virtual list<cacherow_stval> sample(cacherow_stval beliefIndex)=0; // modified for factored, prevly, virtual list<int> sample(int beliefIndex)=0;
		virtual void samplePrepare(cacherow_stval beliefIndex_Sval); // modified for factored, prevly, void samplePrepare(int beliefIndex);
		virtual void samplePrepare(BeliefTreeNode* cn);

		//tree related methods
		//XXX ???
		virtual BeliefTreeNode* getNode(SharedPointer<BeliefWithState>& s); // modified for factored, prevly, BeliefTreeNode* getNode(const state_vector& s);
		virtual void expand(BeliefTreeNode& cn);
		
		virtual BeliefForest* getGlobalNode(void);

		//destructor
		virtual ~Sample(void){}


	};

}; // namespace momdp

#endif

/***************************************************************************
* REVISION HISTORY:
*
***************************************************************************/

