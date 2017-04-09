#include "StateTransitionXXpY.h"

StateTransitionXXpY::StateTransitionXXpY(void)
{
}

StateTransitionXXpY::~StateTransitionXXpY(void)
{
}

SharedPointer<SparseMatrix> StateTransitionXXpY::getMatrix(int a, int x, int xp)
{
  return matrix[a][x][xp];
}
SharedPointer<SparseMatrix> StateTransitionXXpY::getMatrixTr(int a, int x, int xp)
{
  return matrixTr[a][x][xp];
}
