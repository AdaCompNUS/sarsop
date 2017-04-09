#ifndef Rewards_H
#define Rewards_H

#include "Const.h"
#include "Observations.h"
#include "Actions.h"
#include "States.h"
#include "MathLib.h"
#include "BeliefWithState.h"
#include "IVariable.h"
#include "VariableRelation.h"

using namespace std;
using namespace momdp;
namespace momdp 
{
	class MOMDP;
	class Rewards : public MObject
	{
		friend class MOMDP;
	private:
		vector<SharedPointer<SparseMatrix> > matrix;


	public:
		Rewards(void);
		virtual ~Rewards(void);

		vector<SharedPointer<IVariable> > vars;

		virtual REAL_VALUE getReward(BeliefWithState& belief, int a);
		REAL_VALUE reward(States::iterator& x, States::iterator& y, Actions::iterator& a);
		
		virtual SharedPointer<SparseMatrix> getMatrix(int x);
		//virtual SharedPointer<SparseMatrix> getMatrixTr(int x);

	};
}
#endif


