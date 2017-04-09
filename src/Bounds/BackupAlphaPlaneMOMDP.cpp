#include "BackupAlphaPlaneMOMDP.h"
#include "BeliefCache.h"
#include "AlphaPlanePool.h"
#include "SARSOP.h"
#include "exception" 
#include <list>
#include <vector>
#include <cfloat>

using namespace std;
using namespace momdp;

BackupAlphaPlaneMOMDP::BackupAlphaPlaneMOMDP(void)
{
}

BackupAlphaPlaneMOMDP::~BackupAlphaPlaneMOMDP(void)
{
}
// Should use this function with Bound Set
SharedPointer<AlphaPlane> BackupAlphaPlaneMOMDP::backup(BeliefTreeNode* cn)
{
	SharedPointer<AlphaPlane> newPlaneP (new AlphaPlane());
	//AlphaPlane newPlane;  Inefficient implementation, commented out by Yanzhu 15 Aug 2007

	double lbVal;
	//get a new plane at the belief
	lbVal = getNewAlphaPlane(*newPlaneP, *cn);

	// TODO:: migrate below code to SARSOP::onLowerBoundBackup

	newPlaneP->init(solver->numBackups, cn);//to set its timestamp and put cn in its domination list
	//add the new plane as the best alpha for the belief

	AlphaPlanePool* bound = boundSet->set[cn->s->sval];
	list<SharedPointer<AlphaPlane> >* alphas = bound->dataTable->get(cn->cacheIndex.row).ALPHA_PLANES;
	
	if(alphas->size()>0)
	{
		SharedPointer<AlphaPlane>frontAlpha = alphas->front();
		SARSOPAlphaPlaneTuple *tempTuple = (SARSOPAlphaPlaneTuple *)frontAlpha->solverData;
		tempTuple->certed--;
		alphas->pop_front();
	}
	((SARSOPAlphaPlaneTuple *)newPlaneP->solverData)->certed ++;
	alphas->push_front(newPlaneP);
	// TODO:: migrate the addAlphaPlane(newPlaneP) to algorithm based code

	bound->addAlphaPlane(newPlaneP); 
	bound->beliefCache->getRow( cn->cacheIndex.row)->LB = lbVal;
	bound->dataTable->set(cn->cacheIndex.row).ALPHA_TIME_STAMP = newPlaneP->timeStamp;

	return newPlaneP;
}



/* method: getNewAlphaPlane, saves a n-1 calls to getNewAlphaPlaneQ

*/
double BackupAlphaPlaneMOMDP::getNewAlphaPlane(AlphaPlane& result, BeliefTreeNode& cn)
{
	DEBUG_TRACE( cout << "getNewAlphaPlane" << endl; );
	if(cn.isFringe())
	{
		assert(false);
		cout << "Code bug" << endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		// NOTE: make sure whether to use -DBL_MAX or DBL_MIN
		// DBL_MIN is the smallest Positive number
		// -DBL_MAX is the most negative number
		double maxActionLB = -DBL_MAX;
		int maxAction = 0;
		

		FOR (a, problem->getNumActions())
		{
			const BeliefTreeQEntry& Qa = cn.Q[a];
			double sum = 0.0;

			DEBUG_TRACE( cout << "a " << a << endl; );
			FOR (Xn, Qa.getNumStateOutcomes())
			{
				DEBUG_TRACE( cout << "Xn " << Xn << endl; );

				const BeliefTreeObsState* QaXn = Qa.stateOutcomes[Xn];
				//const BeliefTreeObsState& QaXn = Qa.stateOutcomes[Xn];
				if (NULL != QaXn ) 
				{
					FOR (o, QaXn->getNumOutcomes())
						//FOR (o, QaXn.getNumOutcomes())
					{
						DEBUG_TRACE( cout << "o " << o << endl; );

						BeliefTreeEdge* e = QaXn->outcomes[o];
						//BeliefTreeEdge* e = QaXn.outcomes[o];
						if (NULL != e)
						{
							// child node:
							BeliefTreeNode* childNode = e->nextState;

							// SLOWER, separate out the functions in the statement :
							/*	AlphaPlane bestPlane = alphaPlanePool->getBestAlphaPlane(*(childNode));
							alpha_vector bestalpha = bestPlane->alpha;
							double childLB = inner_prod(bestalpha, childNode->s.bvec);
							*/	
							
							SharedPointer<AlphaPlane> tempAlpha = boundSet->getBestAlphaPlane(*(childNode));
							double childLB = inner_prod(*(tempAlpha->alpha), *(childNode->s->bvec));  
							DEBUG_TRACE( cout << "childLB " << childLB << endl; );
							
							
							boundSet->set[childNode->cacheIndex.sval]->beliefCache->getRow( childNode->cacheIndex.row)->LB = childLB;

	

							//SLOWEST:
							//								double childLB = alphaPlanePool->getLowerBoundValue(childNode->s);


							// 260908 fixed bug, prevly multiplied only by e->obsProb
							// 061008 changed calculation for e->obsProb
							sum += childLB * e->obsProb;
							DEBUG_TRACE( cout << "sum " << sum << endl; );
							//sum += childLB * e->obsProb * QaXn->obsStateProb;
						}
						else
						{

						}
					}
				}
			}
			sum *= problem->getDiscount();
			sum += cn.Q[a].immediateReward;
			cn.Q[a].lbVal = sum;
			DEBUG_TRACE( cout << "sum " << sum << endl; );

			DEBUG_TRACE( cout << "maxActionLB " << maxActionLB << endl; );
			if(sum > maxActionLB)
			{
				DEBUG_TRACE( cout << "maxActionLB TO " << sum << endl; );
				maxActionLB = sum;
				maxAction = a;
			}

		}
		assert(maxActionLB !=  -DBL_MAX);
		

		getNewAlphaPlaneQ(result, cn, maxAction);
	

		DEBUG_TRACE(cout << "resulting alpha : " << endl);
		DEBUG_TRACE(result.alpha->write(cout) << endl; );
		return maxActionLB;
	}
}


// lower bound on long-term reward for taking action a (alpha vector)
void BackupAlphaPlaneMOMDP::getNewAlphaPlaneQ(AlphaPlane& result, const BeliefTreeNode& cn, int a)
{
	SharedPointer<alpha_vector> betaA (new alpha_vector(problem->getBeliefSize()));
	SharedPointer<alpha_vector> betaAXnO;
	// use pointer instead of creating a new alpha vector, Yanzhu
	alpha_vector tmp, tmp1, tmp2;

	bool defaultIsSet;
	SharedPointer<BeliefWithState> dummy_stval (new BeliefWithState());

	
	state_val Xc = cn.s->sval;	// state value at current time step (ie X, not X')
	SharedPointer<alpha_vector> defaultBetaAXnO;
	const BeliefTreeQEntry& Qa = cn.Q[a];


	FOR (Xn, Qa.getNumStateOutcomes())
	{
		if( !(problem->XTrans->getMatrix(a, Xc)->isColumnEmpty(Xn)))
		{
			defaultIsSet = false;

			const BeliefTreeObsState* QaXn = Qa.stateOutcomes[Xn];
			if (NULL != QaXn )
			{	
				FOR (o, QaXn->getNumOutcomes())
				{
					if( !(problem->obsProb->getMatrix(a, Xn)->isColumnEmpty(o)))
					{
						BeliefTreeEdge* e = QaXn->outcomes[o];
						//BeliefTreeEdge* e = QaXn.outcomes[o];
						if (NULL != e)
						{
							betaAXnO = (boundSet->getBestAlphaPlane(*(e->nextState))->alpha);
						}
						else
						{
							if (!defaultIsSet)
							{
								dummy_stval->sval = Xn;
								dummy_stval->bvec = cn.s->bvec;
								defaultBetaAXnO = (boundSet->getBestAlphaPlane(*dummy_stval)->alpha);
								defaultIsSet = true;
							}
							betaAXnO = defaultBetaAXnO;
						}	
						emult_column( tmp, *problem->obsProb->getMatrix(a, Xn), o, *betaAXnO );
						mult( tmp1, *problem->YTrans->getMatrix(a, Xc, Xn), tmp);
						emult_column( tmp2, *problem->XTrans->getMatrix(a, Xc), Xn, tmp1 );
						(*betaA) += tmp2;
					}
				}
			}  
			else 
			{
				// still cycle through all possible observations and transform the vector and add to betaA
				//FOR (o, problem->numObservations) 
				for(Observations::iterator oIter = problem->observations->begin(); oIter != problem->observations->end(); oIter ++)
				{
					int o = oIter.index();
					if( !(problem->obsProb->getMatrix(a, Xn)->isColumnEmpty(o)) )
					{
						if (!defaultIsSet)
						{
							dummy_stval->sval = Xn;
							dummy_stval->bvec = cn.s->bvec;
							defaultBetaAXnO = (boundSet->getBestAlphaPlane(*dummy_stval)->alpha);
							defaultIsSet = true;
						}
						betaAXnO = defaultBetaAXnO;

						emult_column( tmp, *problem->obsProb->getMatrix(a, Xn), o, *betaAXnO );
						mult( tmp1, *problem->YTrans->getMatrix(a, Xc, Xn), tmp);
						emult_column( tmp2, *problem->XTrans->getMatrix(a, Xc), Xn, tmp1 );
						(*betaA) += tmp2;
					}

				}
				//}
			} 
		}
	}
	alpha_vector RaXc;


	copy_from_column( RaXc, *(problem->rewards->getMatrix(Xc)), a );
	(*betaA) *= problem->getDiscount();
	(*betaA) += RaXc;

	result.copyFrom(betaA, a, Xc);
}

