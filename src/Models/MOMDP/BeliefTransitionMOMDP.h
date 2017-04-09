#ifndef BeliefTransitionMOMDP_H
#define BeliefTransitionMOMDP_H


#include "BeliefTransition.h"
#include "BeliefWithState.h"

using namespace std;
using namespace momdp;
namespace momdp 
{
class BeliefTransitionMOMDP :
	public BeliefTransition
{
public:
	BeliefTransitionMOMDP(void);
	virtual ~BeliefTransitionMOMDP(void);

	// Where DenseVector (b_x) and belief_vector (b_y) are given as input instead of a BeliefWithState ((x,b_y)), it's for the case where the initial distribution over x is a belief (and not a delta)
	virtual SharedPointer<BeliefWithState> nextBelief(SharedPointer<BeliefWithState> bp, int a, int o, int obsX );
	virtual SharedPointer<BeliefWithState> nextBelief2(SharedPointer<BeliefWithState> bp, int a, int o, int obsX, SharedPointer<SparseVector>& jspv );
	virtual SharedPointer<BeliefWithState> nextBelief(SharedPointer<belief_vector>& belY, DenseVector& belX, int a, int o, int obsX); // SYL07292010 

};
}

#endif
