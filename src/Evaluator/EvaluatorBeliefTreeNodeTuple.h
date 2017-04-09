#ifndef EvaluatorBeliefTreeNodeTuple_H
#define EvaluatorBeliefTreeNodeTuple_H

#include "Tuple.h"
#include "MathLib.h"
using namespace momdp;

namespace momdp
{

	class EvaluatorBeliefTreeNodeTuple : public Tuple
	{
	public:
		int selectedAction;
		REAL_VALUE actionValue;


		EvaluatorBeliefTreeNodeTuple(void)
		{
			selectedAction = -1;
			actionValue = -1.0;
		}

		virtual ~EvaluatorBeliefTreeNodeTuple(void)
		{
		}
	};

	class EvaluatorAfterActionDataTuple : public Tuple
	{
	public:
		//SharedPointer<belief_vector>  actualActionUpdUnobs;
		//SharedPointer<belief_vector>  actualActionUpdObs;

		SharedPointer<SparseVector> spv;


		EvaluatorAfterActionDataTuple(void)
		{
			//actualActionUpdUnobs = NULL;
			//actualActionUpdObs = NULL;
			spv = NULL;
		}

		virtual ~EvaluatorAfterActionDataTuple()
		{
		}
	};

	class EvaluatorAfterObsDataTuple : public Tuple
	{
	public:

		SharedPointer<obs_prob_vector> opv;


		EvaluatorAfterObsDataTuple(void)
		{

			opv = NULL;
		}

		virtual ~EvaluatorAfterObsDataTuple()
		{
		}
	};
}
#endif

