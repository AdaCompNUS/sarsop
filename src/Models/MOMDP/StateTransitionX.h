#ifndef StateTransitionX_H
#define StateTransitionX_H

#include <vector>
#include "Const.h"
#include "Observations.h"
#include "Actions.h"
#include "States.h"
#include "MathLib.h"
#include "VariableRelation.h"

using namespace std;
using namespace momdp;
namespace momdp 
{
	class MOMDP;
	class StateTransitionX : public MObject
	{
		friend class MOMDP;

	private:
		vector<vector<SharedPointer<SparseMatrix> > > matrix;
		vector<vector<SharedPointer<SparseMatrix> > > matrixTr;

	public:
		StateTransitionX(void);
		virtual ~StateTransitionX(void);

		REAL_VALUE prob(States::iterator& x, States::iterator& y, Actions::iterator& a, States::iterator& xp);

		virtual SharedPointer<SparseMatrix> getMatrix(int a, int x);
		virtual SharedPointer<SparseMatrix> getMatrixTr(int a, int x);
	};
}

#endif

