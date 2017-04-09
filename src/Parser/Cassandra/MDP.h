/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 2.28 $  $Author: duyanzhu $  $Date: 2009/01/07 05:10:04 $
   
 @file    MDP.h
 @brief   No brief

 Copyright (c) 2005-2006, Trey Smith. All rights reserved.

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

#ifndef INCMDP_h
#define INCMDP_h

#include <iostream>
#include <string>
#include <vector>

#include "Const.h"
#include "SparseVector.h"
#include "DenseVector.h"

using namespace momdp;
namespace momdp 
{

struct AbstractBound;

// Represents an MDP where state is continuous, time is discrete,
// actions are discrete, and the possible outcomes of an action form a
// discrete probability distribution.  This data structure can
// represent the belief MDP corresponding to a discrete POMDP.
struct MDP 
{
  int numStateDimensions, numActions;
  REAL_VALUE discount;

  virtual ~MDP(void) {}

  int getNumStateDimensions(void)  { return numStateDimensions; }
  int getNumActions(void)  { return numActions; }
  REAL_VALUE getDiscount(void)  { return discount; }

  // returns the initial state
  virtual  state_vector& getInitialState(void)  = 0;

  // returns true if state is terminal
  virtual bool getIsTerminalState( state_vector& s)  = 0;

  // sets result to be the vector of outcome probabilities when from
  // state s action a is selected
  virtual outcome_prob_vector& getOutcomeProbVector(outcome_prob_vector& result,
						     state_vector& s, int a)
        = 0;

  // sets result to be the next state when from state s action a is
  // selected and outcome o results
  virtual state_vector& getNextState(state_vector& result,  state_vector& s, int a,
				     int o)  = 0;

  // returns the expected immediate reward when from state s action a is selected
  virtual REAL_VALUE getReward( state_vector& s, int a)  = 0;

  // returns a new lower bound or upper bound that is valid for
  // this MDP.  notes:
  // * the resulting bound must be initialized before it is used, and
  //   initialization may take significant computation time.
  // * some types of MDP might not define one of these bounds, which
  //   could be signaled by returning NULL.  so far this hasn't been
  //   explored.
//  virtual AbstractBound* newLowerBound(void)  = 0; //!! may be added back later
//  virtual AbstractBound* newUpperBound(void)  = 0; //!! may be added back later
};

}; // namespace momdp

#endif // INCMDP_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: MDP.h,v $
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
 * Revision 2.20  2009/01/07 04:33:54  duyanzhu
 * APPL 0.3, added Policy Evaluator, added Memory Limit option
 *
 * Revision 2.18  2008/09/17 14:08:35  duyanzhu
 * Fix: Prune now only starts 5 seconds after initialization
 *
 * Revision 2.16  2008/07/16 13:27:45  duyanzhu
 * Bring everything to version 2.16
 *
 * Revision 2.12  2008/07/16 08:38:15  duyanzhu
 * Add CPMemUtils class
 *
 * Revision 2.11  2008/07/16 08:34:48  duyanzhu
 * Added parser memory allocation check. Added release script
 *
 * Revision 2.10  2008/06/14 01:41:16  duyanzhu
 * 14 Jun 08 by Du Yanzhu. Added command line option percentageThreshold
 *
 * Revision 1.5  2007/08/17 01:44:35  duyanzhu
 * Change Alpha vector to DenseVector. Xan adds dump functions. Xan's bound propagation code is also included, but not added to Makefile
 *
 * Revision 1.4  2007/08/16 14:06:41  duyanzhu
 * Undo last commit
 *
 * Revision 1.2  2007/08/10 06:03:05  duyanzhu
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2007/07/24 13:17:47  dyhsu
 * Approximate POMDP Planning Library (APPL)
 *
 * Revision 1.2  2007/03/25 18:13:19  elern
 * revised
 *
 * Revision 1.1  2007/03/22 07:19:16  elern
 * initial revision
 *
 * Revision 1.7  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.6  2006/04/05 21:36:16  trey
 * moved OBS_IS_ZERO_EPS from MDP.h to zmdpCommonDefs.h
 *
 * Revision 1.5  2006/04/04 17:26:29  trey
 * moved OBS_IS_ZERO_EPS to MDP.h because it is used all over
 *
 * Revision 1.4  2006/02/06 19:29:23  trey
 * removed numOutcomes field; some MDPs have a varying number of outcomes depending on state
 *
 * Revision 1.3  2006/02/01 01:09:37  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.2  2006/01/31 20:09:11  trey
 * added generic mechanism for getting bounds
 *
 * Revision 1.1  2006/01/28 03:01:05  trey
 * initial check-in
 *
 *
 ***************************************************************************/

