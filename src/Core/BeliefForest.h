#ifndef BeliefForest_H
#define BeliefForest_H

#include "MObjectUser.h"
#include "Belief.h"
#include "MathLib.h"
#include <vector>

using namespace std;
using namespace momdp;

namespace momdp 
{
	class MObject;
	class BeliefTreeNode;
	class PointBasedAlgorithm;
	class MOMDP;
	class Sample;
	class BeliefCache;


	struct SampleRootEdge 
	{
		double sampleRootProb;
		BeliefTreeNode* sampleRoot;
	};

	class BeliefForest 
	{
	private:
		vector <BeliefCache *> *beliefCacheSet;
		Sample* sampleEngine;
                SharedPointer<MOMDP> problem;

	public:

		BeliefForest();
		virtual ~BeliefForest(void);

		virtual void forcedDelete(SharedPointer<Belief> pointer);

		void setup(SharedPointer<MOMDP> problem, Sample* _sampleEngine, vector <BeliefCache *> *_beliefCacheSet);


		belief_vector obsVar_bvec;		// starting belief for observed variable
		/* belief_vector unobsVar_bvec;		// starting belief for unobserved variable */
		void print();

		vector<SampleRootEdge*> sampleRootEdges;		// actual root nodes used in sampling
		double lbVal, ubVal;			// bounds at this dummy root node

		bool isExpanded(void) const { return !( sampleRootEdges.empty() ); }
		unsigned int getGlobalRootNumSampleroots(void) const { return sampleRootEdges.size(); }
		void globalRootPrepare();

	};

}


#endif

