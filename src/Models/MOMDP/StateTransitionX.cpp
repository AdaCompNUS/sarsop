#include "StateTransitionX.h"
#include "MOMDP.h"

StateTransitionX::StateTransitionX(void)
{
}

StateTransitionX::~StateTransitionX(void)
{
}

// (unobserved states, observed states)
SharedPointer<SparseMatrix> StateTransitionX::getMatrix(int a, int x)
{
	return matrix[a][x];
}
SharedPointer<SparseMatrix> StateTransitionX::getMatrixTr(int a, int x)
{
	return matrixTr[a][x];
}