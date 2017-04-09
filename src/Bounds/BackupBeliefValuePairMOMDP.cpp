#include "Const.h"
#include "BackupBeliefValuePairMOMDP.h"
#include "BeliefTreeNode.h"
#include "BeliefCache.h"
#include "MathLib.h"
#include "MOMDP.h"
#include "SARSOP.h"
#include "BeliefValuePairPool.h"
#include "exception" 

using namespace std;
using namespace momdp;

BackupBeliefValuePairMOMDP::BackupBeliefValuePairMOMDP(void)
{
}

BackupBeliefValuePairMOMDP::~BackupBeliefValuePairMOMDP(void)
{
}

// Should use this function with Bound Set
SharedPointer<BeliefValuePair> BackupBeliefValuePairMOMDP::backup(BeliefTreeNode* cn)
{
	int maxUBAction;

	state_val stateidx = cn->s->sval;

	double newUBVal = getNewUBValue(*cn, &maxUBAction);
	SharedPointer<BeliefValuePair> result = boundSet->addPoint(cn->s, newUBVal);    

	if(maxUBAction < 0)
	{
		cout << "error" << endl;
	}
	//  maybePrune();
	//pruning is done in Prune class later
	double lastUbVal = boundSet->set[stateidx]->beliefCache->getRow(cn->cacheIndex.row)->UB;

	boundSet->set[stateidx]->beliefCache->getRow( cn->cacheIndex.row)->UB = newUBVal;

	boundSet->set[stateidx]->dataTable->set(cn->cacheIndex.row).UB_ACTION = maxUBAction;

	DEBUG_TRACE( cout << "Set UB_ACTION: [ " << stateidx << " / " << cn->cacheIndex.row << " ] = " << maxUBAction << endl; );
	
	// TODO:: cn.lastUbVal = lastUbVal;
	//printf("lastUb: %f, ubVal %f\n", lastUbVal, newUBVal);//for debugging purpose
	//return maxUBAction;
	return result;
}
// upper bound on long-term reward for taking action a
double BackupBeliefValuePairMOMDP::getNewUBValueQ(BeliefTreeNode& cn, int a) 
{
	DEBUG_TRACE( cout << "getNewUBValueQ a " << a << endl; );
	DEBUG_TRACE( cout << "cn->cacheIndex " << cn.cacheIndex.row << " " << cn.cacheIndex.sval << endl; );

	double val = 0;
	BeliefTreeQEntry& Qa = cn.Q[a];
	FOR (Xc, Qa.getNumStateOutcomes()) 
	{
		DEBUG_TRACE( cout << "Xc " << Xc << endl; );
		BeliefTreeObsState* QaXc =  Qa.stateOutcomes[Xc];
		if (NULL != QaXc ) 
		{
			FOR(o, QaXc->getNumOutcomes()) 
			{
				DEBUG_TRACE( cout << "o " << o << endl; );
				BeliefTreeEdge* e = QaXc->outcomes[o];
				if (NULL != e) 
				{
					DEBUG_TRACE( cout << "e!=NULL " << endl; );
					DEBUG_TRACE( cout << "e->nextState->cacheIndex " << e->nextState->cacheIndex.row << " " << e->nextState->cacheIndex.sval <<endl; );

					double ubval = boundSet->getValue(e->nextState->s);

					DEBUG_TRACE( cout << "Next Node: " << e->nextState->cacheIndex.row << " : "  << e->nextState->cacheIndex.sval << " action: " << a << " obs: " << o << " ubval: " << ubval << endl; );

					//26092008 corrected, prevly multiplied by e->obsProb only
					// 061008 changed calculation for obsProb
					val += e->obsProb * ubval;
					//val += e->obsProb * QaXc->obsStateProb * ubval;

					DEBUG_TRACE( cout << "val " << val << endl; );

					//26092008 SYL added - update cache for all children
					boundSet->set[e->nextState->cacheIndex.sval]->beliefCache->getRow( e->nextState->cacheIndex.row)->UB = ubval;
				}
			}
		}
	}

	val = Qa.immediateReward + problem->getDiscount() * val; 
	DEBUG_TRACE( cout << "val " << val << endl; );
	Qa.ubVal = val;


	return val;
}

double BackupBeliefValuePairMOMDP::getNewUBValueSimple(BeliefTreeNode& cn, int* maxUBActionP) 
{
	DEBUG_TRACE( cout << "getNewUBValueSimple: " << endl; );

	double val, maxVal = -99e+20;
	int maxUBAction = -1;
	FOR(a, problem->getNumActions()) 
	{
		DEBUG_TRACE( cout << "a: " << a << endl; );
		val = getNewUBValueQ(cn, a);
		DEBUG_TRACE( cout << "val: " << val << endl; );
		DEBUG_TRACE( cout << "maxVal: " << maxVal << endl; );


		if (val > maxVal ) 
		{
			maxVal = val;
			maxUBAction = a;
			DEBUG_TRACE( cout << "maxUBAction TO: " << maxUBAction << endl; );
		}
	}

	if (NULL != maxUBActionP)
	{
		*maxUBActionP = maxUBAction;
	}

	return maxVal;
}

double BackupBeliefValuePairMOMDP::getNewUBValueUseCache(BeliefTreeNode& cn, int* maxUBActionP) 
{
	DEBUG_TRACE( cout << "getNewUBValueUseCache" <<  endl; );
	// cache upper bound for each action
	DenseVector cachedUpperBound(problem->getNumActions());
	
	for(Actions::iterator aIter = problem->actions->begin(); aIter != problem->actions->end(); aIter ++)
	{
		int a = aIter.index();
		cachedUpperBound(a) = cn.Q[a].ubVal;
	}

	// remember which Q functions we have updated on this call
	//std::vector<bool> updatedAction(problem->getNumActions());
	vector<int> updatedAction(problem->actions->size());

	for(Actions::iterator aIter = problem->actions->begin(); aIter != problem->actions->end(); aIter ++)
	{
		int a = aIter.index();
		updatedAction[a] = false;
	}

	double val;
	int maxUBAction = argmax_elt(cachedUpperBound);

	while (1) 
	{
		DEBUG_TRACE( cout << "cachedUpperBound" << endl; );
		DEBUG_TRACE( cachedUpperBound.write(cout) << endl; );
		DEBUG_TRACE( cout << "maxUBAction " << maxUBAction << endl );


		// do the backup for the best Q
		val = getNewUBValueQ(cn, maxUBAction);
		cachedUpperBound(maxUBAction) = val;
		updatedAction[maxUBAction] = true;

		// the best action may have changed after updating Q
		maxUBAction = argmax_elt(cachedUpperBound);

		// if the best action after the update is one that we have already
		//    updated, we're done
		if (updatedAction[maxUBAction]) break;
	}

	double maxVal = cachedUpperBound(maxUBAction);

	if (NULL != maxUBActionP) 
	{
		*maxUBActionP = maxUBAction;
	}
	return maxVal;
}

double BackupBeliefValuePairMOMDP::getNewUBValue(BeliefTreeNode& cn, int* maxUBActionP)
{
	DEBUG_TRACE( cout << "BackupUpperBoundBVpair::getNewUBValue: " <<  cn.cacheIndex.row << " : " << cn.cacheIndex.sval << endl; );
	    	    
	if (CB_QVAL_UNDEFINED == cn.Q[0].ubVal) 
	{
		DEBUG_TRACE( cout << "getNewUBValue:2" << endl; );
		return getNewUBValueSimple(cn, maxUBActionP);
	} 
	else 
	{
		DEBUG_TRACE( cout << "getNewUBValue:3" << endl; );
		return getNewUBValueUseCache(cn, maxUBActionP);
	}

}