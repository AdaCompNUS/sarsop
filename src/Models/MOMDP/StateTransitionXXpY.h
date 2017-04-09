#ifndef StateTransitionXXpY_H
#define StateTransitionXXpY_H

#include "Const.h"
#include "Observations.h"
#include "Actions.h"
#include "States.h"
#include "MathLib.h"
#include "VariableRelation.h"
#include "StateTransitionY.h"

using namespace std;
using namespace momdp;
namespace momdp 
{
	class MOMDP;
	class StateTransitionXXpY : public StateTransitionY
	{
		friend class MOMDP;		
	private:
		typedef vector<vector<vector<SharedPointer<SparseMatrix> > > > Matrices;
		Matrices matrix;
		Matrices matrixTr;

	public:
		StateTransitionXXpY(void);
		virtual ~StateTransitionXXpY(void);

		virtual SharedPointer<SparseMatrix> getMatrix(int a, int x, int xp);
		virtual SharedPointer<SparseMatrix> getMatrixTr(int a, int x, int xp);
	};
}

#endif

