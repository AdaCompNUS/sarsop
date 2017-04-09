#include "MOMDPLite.h"
#include "BeliefTransitionMOMDPLite.h"

MOMDPLite::MOMDPLite(void)
{
	beliefTransition = new BeliefTransitionMOMDPLite();
	beliefTransition->problem = this;
}

MOMDPLite::~MOMDPLite(void)
{
	delete beliefTransition;
}
