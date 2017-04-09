#include "BeliefValuePairPoolSet.h"
#include <exception>
using namespace std;


REAL_VALUE BeliefValuePairPoolSet::getValue(SharedPointer<BeliefWithState>& bns)
{
	state_val sval = bns->sval;
	double bestVal;
	
	bestVal = set[sval]->getValue(bns->bvec);

	return bestVal;

}