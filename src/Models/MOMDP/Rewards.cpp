#include "Rewards.h"
#include "BeliefWithState.h"
#include "Actions.h"

Rewards::Rewards(void)
{
}

Rewards::~Rewards(void)
{
}
SharedPointer<SparseMatrix> Rewards::getMatrix(int x)
{
	return matrix[x];
}
REAL_VALUE Rewards::getReward(BeliefWithState& b, int a)
{
	int Xc = b.sval; // currrent value for observed state variable
	SharedPointer<belief_vector> Bc = b.bvec; // current belief for unobserved state variable

	if (!(matrix[Xc]->isColumnEmpty(a)))
	{
		return inner_prod_column( *matrix[Xc], a, *Bc );
	}
	else
	{
		return 0;
	}
}
