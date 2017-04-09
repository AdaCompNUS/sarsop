/** 
* This code is released under GPL v2
*/

#ifndef BELIEFCACHEROW_H
#define BELIEFCACHEROW_H	

#include "MathLib.h"
#include "BeliefTreeNode.h"

using namespace std;
using namespace momdp;

namespace momdp
{
	class BeliefCacheRow
	{
		
	public:
		BeliefCacheRow()
		{
			BELIEF = NULL;
			REACHABLE = NULL;
		}

		SharedPointer<belief_vector>  BELIEF; /*the belief vector*/
		//double VALUE; /*the value backup value*/
		REAL_VALUE LB; /*the most recent lb value*/
		REAL_VALUE UB; /*the most recent ub value*/
		BeliefTreeNode* REACHABLE;

	};
}

#endif

