#include "BeliefTransitionMOMDP.h"
#include "MOMDP.h"
#include "BeliefWithState.h"
#include "Observations.h"
#include "exception" 

using namespace std;

BeliefTransitionMOMDP::BeliefTransitionMOMDP(void)
{
}

BeliefTransitionMOMDP::~BeliefTransitionMOMDP(void)
{
}

// SYL07292010 added for belief update when there is b_x and b_y, a, x' and o. Only used during simulation/evaluation
// The output is (x',b'_y)
SharedPointer<BeliefWithState> BeliefTransitionMOMDP::nextBelief(SharedPointer<belief_vector>& belY, DenseVector& belX, int a, int o, int obsX)
{
	SharedPointer<MOMDP> momdpProblem = dynamic_pointer_cast<MOMDP> (problem);
	SharedPointer<BeliefWithState> result (new BeliefWithState());

	DenseVector Bc; 
	copy(Bc, *belY);

	unsigned int xn = obsX;

	// for given a, o and x'
	// for a particular x'
	if (!((momdpProblem->obsProb->getMatrix(a, xn))->isColumnEmpty(o))) 
	{
		SparseVector jspv_sum;  // sum of P(x',y' | x, b_{y|x}, a)  * b_x(x) over values of x
		jspv_sum.resize(momdpProblem->YStates->size());

		// loop over x
		FOR (xc, momdpProblem->XStates->size())
		{
			if (!(belX(xc) == 0)) 
			{
				// for a particular x
				// skip all the calculations to add to jspv_sum if *(momdpProblem->XTrans->getMatrix(a, xc)) is all zero for column xn
				if (!((momdpProblem->XTrans->getMatrix(a, xc))->isColumnEmpty(xn))) 
				{
					DenseVector tmp, tmp1;
					SparseVector jspv;
					
					emult_column( tmp, *(momdpProblem->XTrans->getMatrix(a, xc)), xn, Bc );
	
					mult( tmp1, *(momdpProblem->YTrans->getMatrixTr(a, xc, xn)), tmp );
					copy(jspv, tmp1);  // P(x',y' | x, b_{y|x}, a) for a particular x and x'
	
					// multiply with belX(xc) and add to sum over x values
					jspv *= belX(xc);
					jspv_sum += jspv;
				}
			}
		}

		emult_column( *result->bvec, *(momdpProblem->obsProb->getMatrix(a, xn)), o,  jspv_sum); // P(x',y',o | b_x, b_{y|x}, a)
		
		// check that P(x',y',o | b_x, b_{y|x}, a) is not a zero vector for this particular combination of b, a, o and x'
		if (!(result->bvec->data.size() == 0)) {
			// normalize to get P(y'| x', b_x, b_{y|x}, a, o)
			// the normalization factor is P(x',o | b_x, b_{y|x}, a)
			(*result->bvec) *= (1.0/result->bvec->norm_1());		
		} else  throw runtime_error("In nextBelief(), the combination of b, a, o and x' is not possible.");

	} else  throw runtime_error("In nextBelief(), the combination of b, a, o and x' is not possible."); 
	result->bvec->finalize();
	result->sval = obsX;
	return result;

}


//BeliefWithState& POMDP::getNextBeliefStvalFast(BeliefWithState& result, int a, int o, int Xn, const SparseVector tmp1) const

SharedPointer<BeliefWithState> BeliefTransitionMOMDP::nextBelief(SharedPointer<BeliefWithState> bp, int a, int o, int obsX )
{
	SparseVector jspv;
	SharedPointer<MOMDP> momdpProblem = dynamic_pointer_cast<MOMDP> (problem);
	// TODO:: should cache jspv somethere
	momdpProblem->getJointUnobsStateProbVector(jspv, bp , a, obsX);

	
	//cout << " jspv " << endl;
	//jspv.write(cout) << endl;

	SharedPointer<BeliefWithState> result (new BeliefWithState());
	emult_column( *result->bvec, *(momdpProblem->obsProb->getMatrix(a, obsX)), o,  jspv);
	(*result->bvec) *= (1.0/result->bvec->norm_1());
	
	result->sval = obsX;
	return result;

}


// TODO:: nextBelief2 does not make use of bp, remove it
SharedPointer<BeliefWithState> BeliefTransitionMOMDP::nextBelief2(SharedPointer<BeliefWithState> bp, int a, int o, int obsX, SharedPointer<SparseVector>& jspv )
{
	SharedPointer<MOMDP> momdpProblem = dynamic_pointer_cast<MOMDP> (problem);
	//cout << " jspv " << endl;
	//jspv.write(cout) << endl;

	SharedPointer<BeliefWithState> result (new BeliefWithState());
	emult_column( *result->bvec, *(momdpProblem->obsProb->getMatrix(a, obsX)), o,  *jspv);
	(*result->bvec) *= (1.0/result->bvec->norm_1());
	
	result->sval = obsX;
	return result;

}
