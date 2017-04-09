#include <stdexcept>
#include "MOMDP.h"
#include "Sample.h"
#include "BeliefCache.h"
#include "BeliefForest.h"
#include "BeliefTreeNode.h"
#include "PointBasedAlgorithm.h"
#include "MObject.h"


#include "vector"
using namespace std;


BeliefForest::BeliefForest()
{
	sampleEngine == NULL;
}


BeliefForest::~BeliefForest(void)
{
	// TODO:: finish this
	if(sampleEngine != NULL)
	{
		delete sampleEngine;
	}
	

}
void BeliefForest::print()
{
	for(vector<SampleRootEdge*>::iterator iter1= sampleRootEdges.begin() ; iter1!= sampleRootEdges.end() ; iter1++)
	{
		SampleRootEdge* edge = *iter1;
		if(edge == NULL)
		{
			cout << "NULL" << endl;
		}
		else
		{
			edge->sampleRoot->print();
		}

	}
}

//ADD SYLTAG - this function expands the global root
void BeliefForest::globalRootPrepare(void) 
{
	DEBUG_TRACE( cout << "Sample::globalRootPrepare" << endl; );
	belief_vector rootpv;

	rootpv = obsVar_bvec;
		
	sampleRootEdges.resize(rootpv.size());
	DEBUG_TRACE( cout << "rootpv" << endl; );
	DEBUG_TRACE( rootpv.write(cout) << endl; );

	FOR(r, rootpv.size()) 
	{ 
		double rprob = rootpv(r);
		if (rprob > OBS_IS_ZERO_EPS) 
		{
			SharedPointer<BeliefWithState>  thisRootb_s (new BeliefWithState());
			copy(*thisRootb_s->bvec, *problem->getInitialBeliefY(r));
                        thisRootb_s->bvec->finalize();

			SampleRootEdge* rE = new SampleRootEdge();
			sampleRootEdges[r] = rE;
			rE->sampleRootProb = rprob;
			thisRootb_s->sval = r;
			rE->sampleRoot = sampleEngine->getNode(thisRootb_s);

			double lb = (*beliefCacheSet)[rE->sampleRoot->cacheIndex.sval]->getRow(rE->sampleRoot->cacheIndex.row)->LB;
			//double lb = beliefCache->getRow(root->cacheIndex)->LB;

			rE->sampleRoot->count = 1;//for counting valid path
			//rE->sampleRoot->targetUb.push_back(lb);
		} 
		else 
		{
			sampleRootEdges[r] = NULL;
		}

	}


	// get bounds for the Global Root
	double lbSum =0, ubSum = 0;

	FOR(r, sampleRootEdges.size()) 
	{ 
		SampleRootEdge* eR = sampleRootEdges[r];

		if (NULL != eR) 
		{
			BeliefTreeNode & sn = *eR->sampleRoot;				
			lbSum =+  (eR->sampleRootProb) * (*beliefCacheSet)[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->LB;

			ubSum =+  (eR->sampleRootProb) * (*beliefCacheSet)[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->UB;

			// this seems buggy
			/*lbSum =+  bounds->boundsSet[sn.cacheIndex.sval]->beliefCache->getRow(sn.cacheIndex.row)->LB;

			ubSum =+  (globalRoot->sampleRootEdges[r]->sampleRootProb) * bounds->boundsSet[sn.cacheIndex.sval]->beliefCache->getRow(sn.cacheIndex.row)->UB;*/

		}
	}
	lbVal = lbSum;
	ubVal = ubSum;
}

void BeliefForest::forcedDelete(SharedPointer<Belief> pointer)
{
	throw runtime_error("not implemented");
}

void BeliefForest::setup(SharedPointer<MOMDP> problem, Sample* _sampleEngine, vector <BeliefCache *> *_beliefCacheSet)
{
	this->sampleEngine = _sampleEngine;
	this->beliefCacheSet = _beliefCacheSet;
        this->problem = problem;
        
	// initialize beliefForest
	copy(obsVar_bvec,  *(problem->initialBeliefX));
	// copy(unobsVar_bvec, *(problem->initialBelief));

	// unobsVar_bvec.finalize();
	lbVal = -1;
	ubVal = -1;

}
