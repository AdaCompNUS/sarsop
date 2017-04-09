#ifndef MOMDP_H
#define MOMDP_H

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <stdexcept>

#include "Observations.h"
#include "Actions.h"
#include "States.h"

#include "Rewards.h"

#include "ObservationProbabilities.h"
#include "StateTransitionX.h"
#include "StateTransitionXY.h"
#include "StateTransitionXXpY.h"
#include "StateTransitionY.h"

#include "Belief.h"
#include "BeliefTransition.h"

#include "POMDP.h"
#include "MObject.h"
#include "MathLib.h"
#include "FacmodelStructs.h"
#include "POMDPLayer.h"
#include "State.h"
#include "ObsAct.h"
#include "Cache.h"

using namespace std;
using namespace momdp;
namespace momdp 
{
	class FactoredPomdp;

	class MOMDP : public MObject
	{
	private:
		vector<State> stateList;
		vector<ObsAct> observationList;
		vector<ObsAct> actionList;
		vector<ObsAct> rewardList;


		virtual void deleteMatrixVector(vector<SharedPointer<SparseMatrix> > *m);

	public:
        SharedPointer<SparseVector> initialBeliefY;
        vector<SharedPointer<SparseVector> > initialBeliefYByX;

		Cache<SharedPointer<SparseMatrix> > cache;
				
		MOMDP(void); // default constructor

		
		static SharedPointer<MOMDP> convertMOMDPFromPOMDP(POMDP* pomdpProblem);
		static SharedPointer<MOMDP> convertMOMDPFromPOMDPX(FactoredPomdp* factoredPomdp, bool assumeUnknownFlag,unsigned int probType);

		virtual ~MOMDP(void);

		virtual string ToString();
		// functions
		BeliefTransition* beliefTransition;

		// data

		REAL_VALUE discount;

                SharedPointer<BeliefWithState>  initialBeliefStval;
		SharedPointer<DenseVector> initialBeliefX;

		States*	XStates;
		States*	YStates;
		Actions* actions;
		Observations* observations;

		StateTransitionX* XTrans;
		StateTransitionY* YTrans;
		ObservationProbabilities* obsProb;
		Rewards* rewards;

		// TODO: remove the following pomdpXXX variables
		SharedPointer<SparseMatrix> pomdpR;
		vector<SharedPointer<SparseMatrix> > *pomdpT, *pomdpTtr, *pomdpO;

		vector<vector<int> > isPOMDPTerminalState;

                // If some parts do not support intraslice yet, they can check this and complain
                bool hasIntraslice; 

		inline SharedPointer<SparseVector> getInitialBeliefY(int obsState)
		{
		  if( initialBeliefY != NULL ) {
                    return initialBeliefY;
                  } else {
                    return initialBeliefYByX[obsState];
                  }
                }

                virtual REAL_VALUE getDiscount ()
		{
			return discount;
		}

		virtual bool hasPOMDPMatrices();
		virtual void deletePOMDPMatrices();
		
		virtual obsState_prob_vector& getObsStateProbVector(obsState_prob_vector& result, SharedPointer<belief_vector>& belY, DenseVector& belX, int a);
		virtual obsState_prob_vector& getObsStateProbVector(obsState_prob_vector& result, BeliefWithState& b, int a);
		// Xn: next X
		virtual SparseVector& getJointUnobsStateProbVector(SparseVector& result, SharedPointer<BeliefWithState> b, int a, int Xn);

		virtual int getNumActions();
		virtual int getBeliefSize();

		virtual bool getIsTerminalState(BeliefWithState &b);

		virtual void getObsProbVectorFast(obs_prob_vector& result, int a, int Xn, SparseVector& tmp1);
		virtual void getObsProbVector(obs_prob_vector& result, const BeliefWithState& b, int a, int Xn);
		virtual void getObsProbVector(obs_prob_vector& result, SharedPointer<belief_vector>& belY, obsState_prob_vector& belX, int a, int Xn);

		//
		virtual map<string, string> getActionsSymbols(int actionNum);
		virtual map<string, string> getFactoredObservedStatesSymbols(int stateNum) ;
		virtual map<string, string> getFactoredUnobservedStatesSymbols(int stateNum) ;
		virtual map<string, string> getObservationsSymbols(int observationNum) ;
	};
}
#endif
