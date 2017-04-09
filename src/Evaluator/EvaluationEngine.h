#ifndef EvaluationEngine_H
#define EvaluationEngine_H

#include <vector>
#include <string>
#include "MOMDP.h"
using namespace std;
using namespace momdp;

namespace momdp
{
	class AlphaVectorPolicy;
	class BeliefForest;
	class BeliefTreeNode;
	class BeliefCache;
	class Sample;
	class SolverParams;

	class EvaluationEngine
	{
	private:
		SharedPointer<MOMDP> problem;
		SharedPointer<AlphaVectorPolicy> policy;
		BeliefForest* beliefForest;
		Sample *sampleEngine;
		vector <BeliefCache *> *beliefCacheSet;
		SolverParams * solverParams;

	public:
		EvaluationEngine();
		void setup(SharedPointer<MOMDP> problem, SharedPointer<AlphaVectorPolicy> policy, BeliefForest* _beliefForest, vector <BeliefCache *> *_beliefCacheSet, Sample *_sampleEngine, SolverParams * solverParams);
		/* void performAction(belief_vector& outBelUnobs, belief_vector& outBelObs, int action, const BeliefWithState& belSt) const; */
		void getPossibleObservations(belief_vector& possObs, int action, 	const BeliefWithState& belSt) const;
		string toString();

		double getReward(const BeliefWithState& belst, int action);



		void display(belief_vector& b, ostream& s);
		int runFor(int iters, BeliefWithState& startVec, SparseVector startBeliefX, ofstream* streamOut, double& reward, double& expReward);

		// TODO: migrate this to BeliefCacheSet
		BeliefTreeNode* searchNode(SharedPointer<BeliefWithState>  belief);
		

		virtual ~EvaluationEngine(void);
	};

}
#endif

