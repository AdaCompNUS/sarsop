/********** tell emacs we use -*- c++ -*- style comments *******************
$Revision: 2.28 $  $Author: duyanzhu $  $Date: 2009/01/07 05:10:04 $

@file    decision-tree.h
@brief   Efficient decision tree data structure for MDP/POMDP immediate
rewards.

Copyright (c) 2006, Trey Smith. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License"); you may
not use this file except in compliance with the License.  You may
obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied.  See the License for the specific language governing
permissions and limitations under the License.

***************************************************************************/

#ifndef INCdecision_tree_h
#define INCdecision_tree_h

#include "Const.h"

#ifdef __cplusplus
extern "C" {
#endif 


	/**********************************************************************
	See an example of how to use this library in testDecisionTree.c.

	The decision-tree library efficiently stores a mapping [a,s,s',o] ->
	val, where a is an action, s and s' are states, o is an observation
	(all integers), and val is an immediate reward value (real).

	* Construct the immediate reward mapping by calling dtInit() and
	repeatedly calling dtAdd(), one time for each "entry" in the mapping.

	* Query immediate rewards by calling dtGet().

	* When finished, call dtDeallocate().

	The decision-tree library is intended to be used only by the
	imm-reward library.  (The dtGet() function hides behind the
	getImmediateReward() function in imm-reward.c).
	**********************************************************************/

	/* Initialize the decision-tree library--dimensionality of the model
	must be specified so that tables in the decision tree can be
	allocated appropriately later. */
	extern void dtInit(int numActions, int numStates, int numObservations);

	/* Adds an entry to the decision tree.  Any of the first four arguments
	can be given the value -1 (== WILDCARD_SPEC), indicating a wildcard.
	Later calls to dtAdd() overwrite earlier calls. */
	extern void dtAdd(int action, int cur_state, int next_state, int obs, REAL_VALUE val);

	/* Returns the immediate reward for a particular [a,s,s',o] tuple. */
	extern REAL_VALUE dtGet(int action, int cur_state, int next_state, int obs);

	/* Cleans up all decision tree data structures on the heap. */
	extern void dtDeallocate(void);

	/* Print a textual representation of the decision tree data structure to
	stdout.  Intended for debugging. */
	extern void dtDebugPrint(const char* header);

#ifdef __cplusplus
}
#endif 



#endif // INCdecision_tree_h

/***************************************************************************
* REVISION HISTORY:
* $Log: decision-tree.h,v $
* Revision 2.28  2009/01/07 05:10:04  duyanzhu
* added GES.h
*
* Revision 2.26  2009/01/07 05:05:59  duyanzhu
* added evaluator_win
*
* Revision 2.24  2009/01/07 05:01:32  duyanzhu
* add GES
*
* Revision 2.22  2009/01/07 04:59:00  duyanzhu
* adding in UniqueBeliefHeap
*
* Revision 2.20  2009/01/07 04:33:55  duyanzhu
* APPL 0.3, added Policy Evaluator, added Memory Limit option
*
* Revision 2.18  2008/09/17 14:08:36  duyanzhu
* Fix: Prune now only starts 5 seconds after initialization
*
* Revision 2.16  2008/07/16 13:27:45  duyanzhu
* Bring everything to version 2.16
*
* Revision 2.12  2008/07/16 08:38:15  duyanzhu
* Add CPMemUtils class
*
* Revision 2.11  2008/07/16 08:34:49  duyanzhu
* Added parser memory allocation check. Added release script
*
* Revision 2.10  2008/06/14 01:41:17  duyanzhu
* 14 Jun 08 by Du Yanzhu. Added command line option percentageThreshold
*
* Revision 1.5  2007/08/17 01:44:35  duyanzhu
* Change Alpha vector to DenseVector. Xan adds dump functions. Xan's bound propagation code is also included, but not added to Makefile
*
* Revision 1.4  2007/08/16 14:06:41  duyanzhu
* Undo last commit
*
* Revision 1.1.1.1  2007/07/24 13:17:46  dyhsu
* Approximate POMDP Planning Library (APPL)
*
* Revision 1.2  2007/06/04 10:13:37  elern
* adapted a newer version of parser
*
* Revision 1.3  2006/06/01 16:48:36  trey
* cleaned up comments
*
* Revision 1.2  2006/06/01 15:59:55  trey
* no longer publish unnecessary typedefs in header
*
* Revision 1.1  2006/05/29 04:06:02  trey
* initial check-in
*
*
***************************************************************************/
