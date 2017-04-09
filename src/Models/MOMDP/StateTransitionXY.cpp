#include "StateTransitionXY.h"

StateTransitionXY::StateTransitionXY(void)
{
}

StateTransitionXY::~StateTransitionXY(void)
{
}

SharedPointer<SparseMatrix> StateTransitionXY::getMatrix(int a, int x, int xp)
{
	return matrix[a][x];
}
SharedPointer<SparseMatrix> StateTransitionXY::getMatrixTr(int a, int x, int xp)
{
	return matrixTr[a][x];
}
