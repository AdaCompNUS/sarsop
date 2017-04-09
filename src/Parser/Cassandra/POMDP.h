/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 2.28 $  $Author: duyanzhu $  $Date: 2009/01/07 05:10:04 $
   
 @file    POMDP.h
 @brief   Describes data structure of POMDP object which defines the
 @	  pomdp problems

 Copyright (c) 2002-2005, Trey Smith. All rights reserved.

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

#ifndef POMDP_H
#define POMDP_H

#include <iostream>
#include <string>
#include <vector>

#include "Const.h"
#include "MDP.h"
#include "SparseMatrix.h"
#include "SparseVector.h"

namespace momdp 
{

class POMDP : public MDP 
{
public:
  int numStates, numObservations;
  std::string fileName;//added Xan 12082007

  // initialBelief(s)
  SparseVector initialBelief;

  // R(s,a)
  SparseMatrix R;
  // T[a](s,s'), Ttr[a](s',s), O[a](s',o)
  std::vector<SparseMatrix> T, Ttr, O, Otr;
  //std::vector<bool> isPOMDPTerminalState;
  std::vector<int> isPOMDPTerminalState;

  POMDP(void) {}
  POMDP( std::string& fileName, bool useFastParser = false);

  void readFromFile( std::string& fileName, bool useFastParser = false);
  
  /** added on 29th June ajain
   * @brief this calculates the list of possible observations
   * 
   * @param result the plausible observations being returned
   * @param resultProbs the probability of witnessing an observation
   * @param bel current belief 
   * @param act action taken to reach the belief
   */
  void getPossibleObservations(std::vector<int>& result, std::vector<REAL_VALUE>& resultProbs,  belief_vector& bel, int act) ;

  /** added on 20th june ajain
   * @brief this gets the list of possible actions which can be taken 
   * 
   * @param result the list of action ids
   * @param bel current belief vector
   */
  void getPossibleActions(std::vector<int>& result,  belief_vector bel) ;

  //*******added on 30/03/2007 rn
  //	for the access methods
  //returns the transition matrix for action a
   SparseMatrix& getTransitionMatrix(int a)  ;

  //returns the transposed transition matrix for action a
   SparseMatrix& getTransposedTransitionMatrix(int a) ;

  //returns the observation matrix for action a
   SparseMatrix& getObservationMatrix(int a) ;

  //returns the transposed observation matrix for action a
   SparseMatrix& getTransposedObservationMatrix(int a) ;

  //returns the reward matrix
    SparseMatrix& getRewardMatrix() ;

  //returns the max reward value 24/04/2007
	 REAL_VALUE getMaxReward();
  //*******end added

  // returns the initial belief
   belief_vector& getInitialBelief(void) ;

  // sets result to be the vector of observation probabilities when from
  // belief b action a is selected
  obs_prob_vector& getObsProbVector(obs_prob_vector& result,  belief_vector& b,
				    int a) ;

  // sets result to be the next belief when from belief b action a is
  // selected and observation o is observed
  belief_vector& getNextBelief(belief_vector& result,  belief_vector& b,
			       int a, int o) ;

  // returns the expected immediate reward when from belief b action a is selected
  REAL_VALUE getReward( belief_vector& b, int a) ;

  //AbstractBound* newLowerBound(void) ;
  //AbstractBound* newUpperBound(void) ;

  // POMDP-as-belief-MDP aliases for functions implemented in MDP
  int getBeliefSize(void)  { return getNumStateDimensions(); }
  int getNumObservations(void)  { return numObservations; }
  void setBeliefSize(int beliefSize) { numStateDimensions = beliefSize; }
  void setNumObservations(int _numObservations) { numObservations = _numObservations; }

  // POMDP-as-belief-MDP implementations for virtual functions declared in MDP
   belief_vector& getInitialState(void)  { return getInitialBelief(); }
  bool getIsTerminalState( belief_vector& s) ;
  outcome_prob_vector& getOutcomeProbVector(outcome_prob_vector& result,  belief_vector& b,
					    int a) 
    { return getObsProbVector(result,b,a); }
  state_vector& getNextState(state_vector& result,  belief_vector& s,
			     int a, int o) 
    { return getNextBelief(result,s,a,o); }
  
protected:
  void readFromFileCassandra( std::string& fileName);
  void readFromFileFast( std::string& fileName);
  void readFromFileFast2( std::string& fileName);

  void debugDensity(void);
};

}; // namespace momdp

#endif // INCPOMDP_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: POMDP.h,v $
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
 * Revision 2.18  2008/09/17 14:08:36  duyanzhu
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
 * Revision 2.10  2008/06/14 01:41:17  duyanzhu
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
 * Revision 1.1.1.1  2007/07/24 13:17:50  dyhsu
 * Approximate POMDP Planning Library (APPL)
 *
 * Revision 1.6  2007/06/04 07:58:50  jman
 * compiling error fixed and the runtime error handled...
 * now to get the tests to run ...
 *
 * Revision 1.5  2007/04/24 14:37:37  elern
 * added 'getMaxReward' method for CA maxDepth and delta computations
 *
 * Revision 1.4  2007/03/30 14:23:13  elern
 * Added 'getTransposedObservationMatrix' for POMDP
 *
 * Revision 1.3  2007/03/30 05:44:34  elern
 * the access methods changed to  type
 *
 * Revision 1.2  2007/03/30 03:25:16  elern
 * added access methods for POMDP
 *
 * Revision 1.1  2007/03/22 07:19:16  elern
 * initial revision
 *
 * Revision 1.6  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.5  2006/02/17 18:36:35  trey
 * fixed getIsTerminalState() function so RTDP can be used
 *
 * Revision 1.4  2006/02/06 19:26:09  trey
 * removed numOutcomes from MDP class because some MDPs have a varying number of outcomes depending on state; replaced with numObservations in POMDP class
 *
 * Revision 1.3  2006/02/01 01:09:38  trey
 * renamed pomdpCore
 *
 * Revision 1.7  2006/01/29 00:18:36  trey
 * added POMDP() ructor that calls readFromFile()
 *
 * Revision 1.6  2006/01/28 03:03:23  trey
 * replaced BeliefMDP -> MDP, corresponding changes in API
 *
 * Revision 1.5  2005/11/03 17:45:30  trey
 * moved transition dynamics from HSVI implementation to POMDP
 *
 * Revision 1.4  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.3  2005/10/28 02:51:40  trey
 * added copyright headers
 *
 * Revision 1.2  2005/10/27 22:29:12  trey
 * removed dependence on SmartRef header
 *
 * Revision 1.1  2005/10/27 21:38:16  trey
 * renamed POMDPM to POMDP
 *
 * Revision 1.7  2005/10/21 20:08:41  trey
 * added namespace momdp
 *
 * Revision 1.6  2005/03/10 22:53:32  trey
 * now initialize T matrix even when using sla
 *
 * Revision 1.5  2005/01/27 05:32:02  trey
 * switched to use Ttr instead of T under sla
 *
 * Revision 1.4  2005/01/26 04:10:48  trey
 * modified problem reading to work with sla
 *
 * Revision 1.3  2005/01/21 15:21:19  trey
 * added readFromFileFast
 *
 * Revision 1.2  2004/11/24 20:50:16  trey
 * switched POMDPP to be a pointer, not a SmartRef
 *
 * Revision 1.1  2004/11/13 23:29:44  trey
 * moved many files from hsvi to common
 *
 * Revision 1.2  2004/11/09 21:31:59  trey
 * got pomdp source tree into a building state again
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.5  2003/09/16 00:57:02  trey
 * lots of performance tuning, mostly fixed rising upper bound problem
 *
 * Revision 1.4  2003/09/11 01:46:42  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.3  2003/09/07 02:28:07  trey
 * started to adapt for boost matrix library
 *
 * Revision 1.2  2003/07/16 16:07:36  trey
 * added isTerminalState
 *
 * Revision 1.1  2003/06/26 15:41:22  trey
 * C++ version of pomdp solver functional
 *
 *
 ***************************************************************************/
