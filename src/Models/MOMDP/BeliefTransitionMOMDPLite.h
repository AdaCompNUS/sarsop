#ifndef BeliefTransitionMOMDPLite_H
#define BeliefTransitionMOMDPLite_H


#include "BeliefTransition.h"
#include "BeliefWithState.h"
using namespace std;
using namespace momdp;
namespace momdp 
{
class BeliefTransitionMOMDPLite :
	public BeliefTransition
{
public:
	BeliefTransitionMOMDPLite(void);
	virtual ~BeliefTransitionMOMDPLite(void);

	virtual SharedPointer<BeliefWithState> nextBelief(SharedPointer<BeliefWithState> bp, int a, int o, int obsX );
	virtual SharedPointer<BeliefWithState> nextBelief2(SharedPointer<BeliefWithState> bp, int a, int o, int obsX, SharedPointer<SparseVector>& jspv );
	virtual SharedPointer<BeliefWithState> nextBelief(SharedPointer<belief_vector>& belY, DenseVector& belX, int a, int o, int obsX); // SYL07292010 

};
}

#endif
