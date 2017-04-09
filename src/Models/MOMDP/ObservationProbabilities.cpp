#include "ObservationProbabilities.h"

ObservationProbabilities::ObservationProbabilities(void)
{
}

ObservationProbabilities::~ObservationProbabilities(void)
{
}


SharedPointer<SparseMatrix> ObservationProbabilities::getMatrix(int a, int x)
{
	return matrix[a][x];
}
SharedPointer<SparseMatrix> ObservationProbabilities::getMatrixTr(int a, int x)
{
	return matrixTr[a][x];
}

