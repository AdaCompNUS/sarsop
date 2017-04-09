/** 
 * Part of the this code is derived from ZBeliefTree: http://www.cs.cmu.edu/~trey/zmdp/
 * ZBeliefTree is released under Apache License 2.0
 * The rest of the code is released under GPL v2 
 */

/******************************************************************************
 * File: Sample.cc
 * Date: 30/06/2007
 * Author: rn
 * Brief: Super class of all sample classes
 ******************************************************************************/

#include "Sample.h"
#include "BeliefCache.h"
#include "AlphaPlane.h"
#include "AlphaPlaneMaxMeta.h"
#include "GlobalResource.h"

//#define DEBUGSYL_310708 1
//#define DEBUGSYLTAG 1

using namespace std;
using namespace momdp;

namespace momdp
{

     /***************************************
       BASIC REACHABILITY TREE METHODS
      ****************************************/
     void Sample::samplePrepare(cacherow_stval beliefIndex_Sval)
     {
	  state_val stateidx = beliefIndex_Sval.sval;
	  int	row =  beliefIndex_Sval.row;

	  //do the preparation work before sample
	  BeliefTreeNode* cn = (*beliefCacheSet)[stateidx]->getRow(row)->REACHABLE;
	  //BeliefTreeNode* cn = beliefCache->getRow(row)->REACHABLE;

	  if(cn->isFringe())
	  {
	       expand(*cn);
	  }
     }

     void Sample::samplePrepare(BeliefTreeNode* cn)
     {
	  if(cn->isFringe())
	  {
	       expand(*cn);
	  }
     }

     //ADD SYLTAG - this function just creates, initializes and returns pointer. It does not expand.
     BeliefForest* Sample::getGlobalNode(void) 
     {
	  return beliefForest;
     }




     BeliefTreeNode* Sample::getNode(SharedPointer<BeliefWithState>& b_s) 
     {
	  SharedPointer<belief_vector>& s = b_s->bvec;
	  state_val stateidx = b_s->sval;

	  bool keepLowerBound = true;

	  DEBUG_TRACE( cout << "Sample::getNode stateidx " << stateidx; );
	  DEBUG_TRACE( cout << " s" << endl; );
	  DEBUG_TRACE( s->write(cout) << endl; );
	  DEBUG_TRACE( cout << " hash: " << s->md5HashValue() << endl; );

	  int row = (*beliefCacheSet)[stateidx]->getBeliefRowIndex(s);

	  DEBUG_TRACE( cout << "testedRowIndex: " << row << endl; );


	  bool isTerminal;
	  double ubVal, lbVal;

	  if (row==-1) 
	  {
	       // create a new fringe node
	       BeliefTreeNode* cn = new BeliefTreeNode();
	       int timeStamp = -1;//for initializing timeStamp of the beliefNode
	       cn->s = b_s;

	       cn->count = 0; //for SARSOP
	       cn->checked = false;

	       row = (*beliefCacheSet)[stateidx]->addBeliefRowWithoutCheck(cn->s->bvec);

	       cn->cacheIndex.row = row;
	       cn->cacheIndex.sval = cn->s->sval;

	       isTerminal = problem->getIsTerminalState(*b_s);

	       (*beliefCacheSet)[stateidx]->getRow( row)->REACHABLE = cn;
	       // TODO:: check if anyone using this: solver->beliefCacheSet[stateidx]->getRow( row)->IS_TERMINAL = isTerminal;

	       for(int i = 0 ; i < onGetNode.size(); i++)
	       {
		    (*onGetNode[i])(solver, cn, b_s);
	       }



	       DEBUG_TRACE( cout << "cn->cacheIndex.row " << cn->cacheIndex.row << " count " << cn->count << endl; );



	       return cn;
	  }
	  else 
	  {
	       // return existing node
	       BeliefTreeNode* temp = (*beliefCacheSet)[stateidx]->getRow(row)->REACHABLE;
	       return temp;
	  }
     }

     //Assumption: cn.depth is defined already
     void Sample::expand(BeliefTreeNode& cn)
     {
	  DEBUG_TRACE( cout<<"expand"<< endl; );
	  DEBUG_TRACE( cout << "cn.s sval " << cn.s->sval << " index " << cn.cacheIndex.row << endl; );
	  DEBUG_TRACE( cn.s->bvec->write(cout) << endl; );

	  // set up successors for this fringe node (possibly creating new fringe nodes)
	  obs_prob_vector  opv; // outcome_prob_vector opv;
	  //state_vector *sp = new state_vector;
	  SharedPointer<BeliefWithState> sp; // state_vector sp;
	  obsState_prob_vector spv;  // outcome probability for values of observed state
	  SharedPointer<SparseVector> jspv (new SparseVector());

	  //obs_prob_vector  ospv; // outcome_prob_vector opv;

	  cn.Q.resize(problem->getNumActions());

	  // list<int>::iterator iter;		doesnt seem to be used

	  for(Actions::iterator aIter =  problem->actions->begin() ; aIter != problem->actions->end(); aIter ++)
	  {
	       int a = aIter.index();
	       DEBUG_TRACE( cout << "Sample::expand a " << a << endl; );
	       BeliefTreeQEntry& Qa = cn.Q[a];
	       //Qa.immediateReward = problem->getReward(cn.s, a);
	       Qa.immediateReward = problem->rewards->getReward(*cn.s, a);


	       problem->getObsStateProbVector(spv, *(cn.s), a); // P(Xn|cn.s,a)
	       Qa.stateOutcomes.resize(spv.size());
	       Qa.valid = true;



	       DEBUG_TRACE( cout << "spv" << endl; );
	       DEBUG_TRACE( spv.write(cout) << endl; );

	       for(States::iterator xIter =  problem->XStates->begin() ; xIter != problem->XStates->end(); xIter ++)
	       {
		    int Xn = xIter.index();

		    DEBUG_TRACE( cout << "Sample::expand Xn " << Xn << endl; );

		    double sprob = spv(Xn);
		    if (sprob > OBS_IS_ZERO_EPS) 
		    {
			 BeliefTreeObsState* xe = new BeliefTreeObsState();
			 Qa.stateOutcomes[Xn] = xe;
			 problem->getJointUnobsStateProbVector(*jspv, (SharedPointer<BeliefWithState> )cn.s, a, Xn);
			 //problem->getStatenObsProbVectorFast(ospv, a, Xn, jspv);
			 problem->getObsProbVectorFast(opv, a, Xn, *jspv); // only the joint prob is useful for later but we calculate the observation prob P(o|Xn,cn.s,a)
			 //problem->getObsProbVector(opv, cn.s, a, Xn);
			 xe->outcomes.resize(opv.size());

			 for(Observations::iterator oIter =  problem->observations->begin() ; oIter != problem->observations->end(); oIter ++)
			 {
			      //FOR(o, opv.size()) 
			      int o = oIter.index();

			      DEBUG_TRACE( cout << "Sample::expand o " << o << endl; );

			      double oprob = opv(o);
			      if (oprob > OBS_IS_ZERO_EPS) 
			      {
				   BeliefTreeEdge* e = new BeliefTreeEdge();
				   xe->outcomes[o] = e;
				   //QaXn.outcomes[o] = e;
				   //e->obsProb = oprob;
				   e->obsProb = oprob * sprob; // P(o,Xn|cn.s,a) = P(Xn|cn.s,a) * P(o|Xn,cn.s,a)
				   //e->nextState = getNode(problem->getNextBeliefStvalFast(sp, a, o, Xn, jspv));
				   sp = (problem->beliefTransition->nextBelief2(cn.s, a, o, Xn, jspv)) ;
				   sp->bvec->finalize();
				   e->nextState = getNode(sp);

				   DEBUG_TRACE( cout << "Sample::expand e->nextState row " << e->nextState->cacheIndex.row << endl; );
				   DEBUG_TRACE( cout << "Sample::expand e->nextState belief " << endl; );
				   DEBUG_TRACE( e->nextState->s->bvec->write(cout) << endl; );

				   //e->nextState = getNode(problem->getNextBeliefStval(sp, cn.s, a, o, Xn));
				   e->nextState->count++;//increment valid-path count, for the current

				   DEBUG_TRACE( cout << "e->nextState->cacheIndex.row " << e->nextState->cacheIndex.row << " count " << e->nextState->count << endl; );
				   // new path which runs into the node
			      }
			      else 
			      {
				   xe->outcomes[o] = NULL;
			      }
			 }
		    } 
		    else 
		    { 
			 Qa.stateOutcomes[Xn] = NULL;
		    }
	       }
	       Qa.ubVal = CB_QVAL_UNDEFINED;
	  }
	  numStatesExpanded++;
     }

};
