#include "SimulationEngine.h"
#include "AlphaVectorPolicy.h"
#include "solverUtils.h"
#include "Controller.h"

using namespace std;
using namespace momdp;

namespace momdp
{
    Controller::Controller(SharedPointer<MOMDP> problem,
                           SharedPointer<AlphaVectorPolicy> policy,
                           SolverParams* solverParams,
                           int initialBeliefStvalX):
            problem(problem),
            policy(policy),
            solverParams(solverParams),
            currBelSt(new BeliefWithState()),
            firstAction(true)
    {
        // Initialize the starting state of X
        currBelSt->sval = initialBeliefStvalX == -1 ?
                problem->initialBeliefStval->sval :
                initialBeliefStvalX;

        // Initialize the belief vector for Y
        SharedPointer<SparseVector> startBeliefVec;
        if (problem->initialBeliefStval->bvec)
            startBeliefVec = problem->initialBeliefStval->bvec;
        else
            startBeliefVec = problem->initialBeliefYByX[currBelSt->sval];

        copy(*currBelSt->bvec, *startBeliefVec);
    }

    int const Controller::nextAction(ObsDefine currObservation,
                                     int nextStateX)
    {
        if (firstAction) {
            firstAction = false;
        }
        else
        {
            // If this is not the first action, we first update the
            // belief according to the last action and the current
            // observation

            // Next belief with state
            SharedPointer<BeliefWithState> nextBelSt;

            // Create next belief
            nextBelSt = problem->beliefTransition->nextBelief(currBelSt, lastAction, currObservation, nextStateX);

            // Assign to current belief
            copy(*currBelSt->bvec, *nextBelSt->bvec);
            currBelSt->sval = nextBelSt->sval;
        }

        // Belief conditioning on X. May not be used.
        DenseVector currBelX;

        int currAction;
        if (solverParams->useLookahead)
            currAction = policy->getBestActionLookAhead(*currBelSt);
        else
            currAction = policy->getBestAction(*currBelSt);

        if (currAction < 0)
        {
            cerr << "You are using a MDP Policy, please make sure you are using a MDP policy together with one-step look ahead option turned on" << endl;
            return -1;
        }

        lastAction = currAction;

        return currAction;
    }

    SharedPointer<BeliefWithState> Controller::currBelief() const
    {
        return currBelSt;
    }
}
