#ifndef EvaluatorSampleEngine_H
#define EvaluatorSampleEngine_H


#include <list>
#include <vector>
#include "Belief.h"
#include "Sample.h"
#include "BeliefTreeNode.h"
#include "BeliefForest.h"
#include "PointBasedAlgorithm.h"
using namespace std;
using namespace momdp;

namespace momdp
{
	class MOMDP;


	class EvaluatorSampleEngine : public Sample
	{
	public:
		EvaluatorSampleEngine()
		{
			numStatesExpanded = 0;
		}

		virtual void setup(PointBasedAlgorithm* _solver, SharedPointer<MOMDP> _problem, vector <BeliefCache *> *_beliefCacheSet, BeliefForest* _beliefForest)
		{
			this->solver = _solver;
			this->problem = _problem;
			this->beliefCacheSet = _beliefCacheSet;
			this->beliefForest = _beliefForest;
		}
		
			
		virtual list<cacherow_stval> sample(cacherow_stval beliefIndex, unsigned int currentRoot) // modified for factored, prevly, virtual list<int> 
		{
			return list<cacherow_stval>();
		}

		//virtual list<cacherow_stval> sample(cacherow_stval beliefIndex)=0; // modified for factored, prevly, virtual list<int> sample(int beliefIndex)=0;
		virtual void samplePrepare(cacherow_stval beliefIndex_Sval); // modified for factored, prevly, void samplePrepare(int beliefIndex);
		virtual void samplePrepare(BeliefTreeNode* cn);

		//tree related methods
		virtual BeliefTreeNode* getNode(SharedPointer<BeliefWithState>& s); // modified for factored, prevly, BeliefTreeNode* getNode(const state_vector& s);
		virtual void expand(BeliefTreeNode& cn);
		
		virtual BeliefForest* getGlobalNode(void);

		//destructor
		virtual ~EvaluatorSampleEngine(void){}


	};

}; // namespace momdp


#endif
