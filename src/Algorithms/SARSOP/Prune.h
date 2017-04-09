/** 
* Part of the this code is derived from ZMDP: http://www.cs.cmu.edu/~trey/zmdp/
* ZMDP is released under Apache License 2.0
* The rest of the code is released under GPL v2 
*/

/********** tell emacs we use -*- c++ -*- style comments *******************
$Revision: 2.10 $  $Author: duyanzhu $  $Date: 2008/06/14 01:41:13 $

@file    Prune.h
@brief   the superclass of all pruning classes

***************************************************************************/
#ifndef PRUNE_H
#define PRUNE_H

#include <list>
#include <vector>
#include "Belief.h"
#include "BeliefTreeNode.h"
#include "BeliefForest.h"
#include "PointBasedAlgorithm.h"
using namespace std;
using namespace momdp;

namespace momdp
{

	class Prune
	{
	public:
		//fields
		SharedPointer<MOMDP> problem;
		PointBasedAlgorithm* solver;

		Prune( PointBasedAlgorithm* _solver)
		{
			solver = _solver;
			problem = solver->problem;
		}

		//methods
		virtual void prune(void)=0;

		virtual ~Prune(){}
	};

}; // namespace momdp

#endif
/***************************************************************************
* REVISION HISTORY:
*
***************************************************************************/

