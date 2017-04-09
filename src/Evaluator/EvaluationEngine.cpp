#include <sstream>
#include <fstream>
#include "EvaluationEngine.h"
#include "AlphaVectorPolicy.h"
#include "CPTimer.h"
#include "BeliefForest.h"
#include "Sample.h"
#include "BeliefCache.h"
#include "solverUtils.h"

#include "EvaluatorBeliefTreeNodeTuple.h"

using namespace std;
using namespace momdp;

namespace momdp
{
	EvaluationEngine::EvaluationEngine()
	{
	}
	void EvaluationEngine::setup(SharedPointer<MOMDP> problem, SharedPointer<AlphaVectorPolicy> policy, BeliefForest* _beliefForest,vector <BeliefCache *> *_beliefCacheSet,  Sample *_sampleEngine, SolverParams * solverParams)
	{
		this->policy = policy;
		this->problem = problem;
		this->beliefForest = _beliefForest;
		this->beliefCacheSet = _beliefCacheSet;
		this->sampleEngine = _sampleEngine;
		this->solverParams = solverParams;
	}

	EvaluationEngine::~EvaluationEngine(void)
	{
	}

  /*	void EvaluationEngine::performAction(belief_vector& outBelUnobs, belief_vector& outBelObs, int action, const BeliefWithState& belSt) const 
	{
		// DEBUG_SIMSPEED_270409 skip calculating outprobs for x when there is only one possible x value
		if (problem->XStates->size() == 1) 
		{
			// clear out the entries
			outBelObs.resize(1);
			outBelObs.push_back(0,1.0); 
		} 
		else 
		{
			//problem->getTransitionMatrixX(action, belSt.sval);
			const SharedPointer<SparseMatrix>  transMatX = problem->XTrans->getMatrix(action, belSt.sval); 
			mult(outBelObs, *belSt.bvec, *transMatX);
		}

		//const SparseMatrix transMatY = problem->getTransitionMatrixY(action, belSt.sval);
		const SharedPointer<SparseMatrix>  transMatY = problem->XYTrans->getMatrix(action, belSt.sval);
		mult(outBelUnobs, *belSt.bvec, *transMatY);
	}
  */

	void EvaluationEngine::getPossibleObservations(belief_vector& possObs, int action, 	const BeliefWithState& belSt) const
	{
		//const SparseMatrix obsMat = problem->getObservationMatrix(action, belSt.sval);
		const SharedPointer<SparseMatrix>  obsMat = problem->obsProb->getMatrix(action, belSt.sval);
		mult(possObs,  *belSt.bvec, *obsMat);
	}


	double EvaluationEngine::getReward(const BeliefWithState& belst, int action)
	{
		//const SparseMatrix rewMat = problem->getRewardMatrix(belst.sval);
		const SharedPointer<SparseMatrix>  rewMat = problem->rewards->getMatrix(belst.sval);
		return inner_prod_column(*rewMat, action, *belst.bvec);
	}

	string EvaluationEngine::toString()
	{
		std::ostringstream mystrm; 
		mystrm << "action selector: (replaced by Policy) ";
		return mystrm.str();
	}

	void EvaluationEngine::display(belief_vector& b, ostream& s)
	{
		for(unsigned int i = 0; i < b.filled(); i++)
		{
			s << b.data[i].index << " -> " << b.data[i].value << endl;
		}
	}

	BeliefTreeNode* EvaluationEngine::searchNode(SharedPointer<BeliefWithState>  belief)
	{
		SharedPointer<belief_vector> s = belief->bvec;
		state_val stateidx = belief->sval;
		
		int beliefIndex = (*beliefCacheSet)[stateidx]->getBeliefRowIndex(s);

		if (beliefIndex==-1) 
		{
			return NULL;
		}
		else
		{
			// return existing node
			BeliefTreeNode* temp = (*beliefCacheSet)[stateidx]->getRow(beliefIndex)->REACHABLE;
			return temp;
		}
	}

	int EvaluationEngine::runFor(int iters, BeliefWithState& startVec, SparseVector startBeliefX, ofstream* streamOut, double& reward, double& expReward)
	{ 
		DEBUG_TRACE(cout << "runFor" << endl; );
		DEBUG_TRACE(cout << "iters " << iters << endl; );
		DEBUG_TRACE(cout << "startVec sval " << startVec.sval << endl; );
		DEBUG_TRACE(startVec.bvec->write(cout) << endl;);
		DEBUG_TRACE(cout << "startBeliefX" << endl; );
		DEBUG_TRACE(startBeliefX.write(cout) << endl;);


		//    double reward = 0, expReward = 0;
		bool enableFiling = false;
		if(streamOut == NULL)
		{
			enableFiling = false;
		}
		else 
		{
			enableFiling = true;
		}

		BeliefTreeNode* curNode = NULL;
		if(startVec.sval == -1)
		{
	       		cerr << "Please use the simulator. Random initial value for the fully observed state variable is not supported in the evaluator.\n";
	       		exit(1);
			int sampledX = chooseFromDistribution(startBeliefX);
			curNode = beliefForest->sampleRootEdges[sampledX]->sampleRoot;
		}
		else
		{
			curNode = beliefForest->sampleRootEdges[startVec.sval]->sampleRoot;
		}

		// policy follower state
		// belief with state
		SharedPointer<BeliefWithState> currBelSt = curNode->s;

		DEBUG_TRACE( cout << "currBelSt sval " << currBelSt->sval << endl; );
		DEBUG_TRACE( currBelSt->bvec->write(cout) << endl; );

		double mult=1;
		CPTimer lapTimer;

		unsigned int firstAction;

		double gamma = problem->getDiscount();
		for(int timeIter = 0; timeIter < iters; timeIter++)
		{ 
			DEBUG_TRACE( cout << "timeIter " << timeIter << endl; );

			if(enableFiling && timeIter == 0)
			{
				*streamOut << ">>> begin\n";
			}

			// get action according to policy and current belief and state
			int currAction = -1;
			
			sampleEngine->samplePrepare(curNode);
			EvaluatorBeliefTreeNodeTuple *curNodeExtraData = (EvaluatorBeliefTreeNodeTuple *)curNode->extraData;

			if(curNodeExtraData->selectedAction == -1)
			{
				// note, previous versions treated the first time step differently when there is random initial value for the fully observed state variable
				// here, we assume that in such cases, the initial value for the fully observed state variable is sampled, and the policy follower knows the sampled value
				if(solverParams->useLookahead)
				{
					currAction = policy->getBestActionLookAhead(*currBelSt);
				}
				else
				{
					currAction = policy->getBestAction(*currBelSt);
				}

				curNodeExtraData->selectedAction = currAction;
			} 
			
			currAction = curNodeExtraData->selectedAction;

			if(currAction < 0 )
			{
				cout << "You are using a MDP Policy, please make sure you are using a MDP policy together with one-step look ahead option turned on" << endl;
				return -1;
			}

			if (timeIter == 0)
			{
				firstAction = currAction;
			}

			// this is the reward for the "actual state" system
			//double currReward = getReward(actStateCompl, currAction);
			double currReward = curNode->Q[currAction].immediateReward;

			DEBUG_TRACE( cout << "currAction " << currAction << endl; );

			DEBUG_TRACE( cout << "currReward " << currReward << endl; );
			expReward += mult*currReward;
			mult *= gamma;
			reward += currReward;

			DEBUG_TRACE( cout << "expReward " << expReward << endl; );
			DEBUG_TRACE( cout << "reward " << reward << endl; );

			EvaluatorAfterActionDataTuple *afterActionDataTuple = (EvaluatorAfterActionDataTuple *)curNode->Q[currAction].extraData;
			
		
			// the actual next state for the observed variable
			int newObsState = (unsigned int) chooseFromDistribution(*afterActionDataTuple->spv);

			EvaluatorAfterObsDataTuple* afterObsDataTupel = (EvaluatorAfterObsDataTuple*)curNode->Q[currAction].stateOutcomes[newObsState]->extraData;
			SharedPointer<obs_prob_vector> obsPoss = afterObsDataTupel->opv;
			//getPossibleObservations(obsPoss, currAction, actNewStateCompl);  

		
			DEBUG_TRACE( cout << "obsPoss"<< endl; );
			DEBUG_TRACE( obsPoss->write(cout) << endl; );

			int currObservation = chooseFromDistribution(*obsPoss);

			DEBUG_TRACE( cout << "currObservation "<< currObservation << endl; );

			if(enableFiling)
			{

				if (timeIter == 0) 
				{
					*streamOut<<"X: "<<curNode->s->sval << endl; 

					*streamOut << "belief (over Y): " ;

					std::vector<SparseVector_Entry>::const_iterator iter;
					for(iter = curNode->s->bvec->data.begin(); iter != curNode->s->bvec->data.end(); iter++)
					{
						*streamOut << iter->index <<":"<<iter->value << " ";
					}
					*streamOut << endl;
				}

				*streamOut<<"A: "<<currAction<< endl; 
				*streamOut<<"R: "<<currReward<< endl; 
				*streamOut<<"X: "<<newObsState << endl; 
				*streamOut<<"O: "<<currObservation<< endl; 

			}

			// now that we have the action, state of observed variable, and observation,
			// we can update the belief of unobserved variable
			// this is for the tree, i.e. the "actual system"
			curNode = curNode->Q[currAction].stateOutcomes[newObsState]->outcomes[currObservation]->nextState;

			currBelSt = curNode->s;  

			if(enableFiling)
			{
				if(timeIter == iters - 1)
				{
					*streamOut << "terminated\n";
				}

				*streamOut << "belief (over Y): " ;

				std::vector<SparseVector_Entry>::const_iterator iter;
				for(iter = curNode->s->bvec->data.begin(); iter != curNode->s->bvec->data.end(); iter++)
				{
					*streamOut << iter->index <<":"<<iter->value << " ";
				}
				*streamOut << endl;

				if(timeIter == iters - 1)
				{
					//timeval timeInRunFor = getTime() - prevTime;				
					//*streamOut << "----- time: " << timevalToSeconds(timeInRunFor) <<endl;
					double lapTime = lapTimer.elapsed();
					*streamOut << "----- time: " << lapTime <<endl;

				}
			} 
			
			// added to stop simulation when at terminal state
			//if(problem->getIsTerminalState(*currBelSt))
			if(problem->getIsTerminalState(*curNode->s))
			{
				// Terminal state
				// reward all 0, transfer back to it self
				// TODO Consider reward not all 0 case
				//cout <<"Terminal State!! timeIter " << timeIter << endl;
				if(enableFiling)
				*streamOut << "Reached terminal state" << endl;
				break;
				break;
			}



		}

		return firstAction;
	}
};
