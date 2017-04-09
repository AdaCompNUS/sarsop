#ifndef POMDPLAYER_H
#define POMDPLAYER_H

// the following are codes to handle different cases
// they are here as OfflineEngine.cc, POMDP.cc and
// FactoredPomdp.cc require them
// Added by Won Kok Sung 11 Feb 2009
#define FULLY_OBSERVED 0
#define FULLY_UNOBSERVED 1
#define MIXED 2
#define MIXED_REPARAM 3

using namespace std;

class POMDPLayer {
    public:

        std::vector<std::vector<SharedPointer<SparseMatrix> > > TX, TXtr, O, Otr;

        std::vector<std::vector<SharedPointer<SparseMatrix> > > TY, TYtr;
        std::vector<std::vector<std::vector<SharedPointer<SparseMatrix> > > > TY_reparam, TYtr_reparam;

        std::vector<std::vector<int> > isPOMDPTerminalState;
        std::vector<SharedPointer<SparseMatrix> > R;

        SharedPointer<SparseMatrix>  terminalStateReward;

        SparseVector initialBeliefX;
        int initialStateX;

        SparseVector initialBeliefY;
        vector<SparseVector> initialBeliefY_reparam;


        /* SparseVector initialBeliefXNew; */
        /* int initialStateXNew; */
        /* SparseVector initialBeliefYNew; */


        double discount;
        int numActions;
        int numObservations;
        int numStatesObs;
        int numStatesUnobs;

        //POMDP information
        // R(s,a)
        SharedPointer<SparseMatrix>  pomdpR;
        //T[a](s,s'), Ttr[a](s',s), O[a](s',o)
        std::vector<SharedPointer<SparseMatrix> > pomdpT, pomdpTtr, pomdpO, pomdpOtr;

        std::vector<int> pomdpIsPOMDPTerminalState;
        SparseVector pomdpInitialBelief;
        double pomdpDiscount;
        int pomdpNumActions, pomdpNumObservations, pomdpNumStates;

		POMDPLayer()
		{
			pomdpR = NULL;

		}

};

#endif
