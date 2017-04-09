#include "BeliefTransitionMOMDPLite.h"
#include "exception" 
#include <stdexcept>
using namespace std;

BeliefTransitionMOMDPLite::BeliefTransitionMOMDPLite(void)
{
}

BeliefTransitionMOMDPLite::~BeliefTransitionMOMDPLite(void)
{
}


SharedPointer<BeliefWithState> BeliefTransitionMOMDPLite::nextBelief(SharedPointer<BeliefWithState> bp, int a, int o, int obsX )
{
	throw runtime_error("not implemented");
}
SharedPointer<BeliefWithState> BeliefTransitionMOMDPLite::nextBelief2(SharedPointer<BeliefWithState> bp, int a, int o, int obsX, SharedPointer<SparseVector>& jspv )
{
	throw runtime_error("not implemented");
}

