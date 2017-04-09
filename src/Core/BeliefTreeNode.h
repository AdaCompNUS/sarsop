#ifndef BeliefTreeNode_H
#define BeliefTreeNode_H

#include <vector>
#include "Const.h"
#include "FacmodelStructs.h"
#include "MObject.h"

using namespace std;
using namespace momdp;
namespace momdp 
{
#define CB_QVAL_UNDEFINED (-99e+20)
#define CB_INITIALIZATION_PRECISION_FACTOR (1e-2)

	class BeliefTreeNode;
	class BeliefCache;
	class Tuple;

	class BeliefTreeEdge 
	{
	public:
		double obsProb;		// note that this is the joint prob of observation and next_x value
		BeliefTreeNode* nextState;
		BeliefTreeEdge();
		~BeliefTreeEdge();
	};
	

	// added for factored - SYL 17072008
	class BeliefTreeObsState 
	{
	public:
		Tuple* extraData;
		vector<BeliefTreeEdge*> outcomes;	
		size_t getNumOutcomes(void) const { return outcomes.size(); }
		BeliefTreeObsState()
		{
			extraData = NULL;
		}
	};

	class BeliefTreeQEntry
	{
	public:
		double immediateReward;
		//std::vector<BeliefTreeEdge*> outcomes;		// removed for factored - SYL 17072008
		vector<BeliefTreeObsState*> stateOutcomes;	// added for factored - SYL 17072008
		Tuple* extraData;
		double lbVal, ubVal;
		bool valid;//added rn 12/18/2006 //for purpose of record which entries are valid (not sub-optimal)
		size_t getNumStateOutcomes(void) const { return stateOutcomes.size(); }
		BeliefTreeQEntry()
		{
			extraData = NULL;
		}
	};

	struct cacherow_stval 
	{
		int 	      row;
		state_val     sval;
	};

	class BeliefTreeNode :	public MObject
	{
	public:
		BeliefTreeNode(void);
		virtual ~BeliefTreeNode(void);


		int count;//the count of valid paths which led to the belief node in the reachable tree
		SharedPointer<BeliefWithState> s;				// added for factored - SYL 17072008
		cacherow_stval cacheIndex;		// added for factored - SYL 17072008

		vector<BeliefTreeQEntry> Q;

		/*double lastUbVal;
		list<int> depth;
		list<int> depth2;
		list<double> targetUb;
		list<bool> examined;*/
		
		void print();

		// TODO: migrate checked to Pruning data tuple
		bool checked;

		bool isFringe(void) const { return Q.empty(); }
		size_t getNodeNumActions(void) const { return Q.size(); }
		BeliefTreeNode& getNextState(int a, int o, int x) { return *Q[a].stateOutcomes[x]->outcomes[o]->nextState; }  // added for factored - SYL 17072008

		Tuple* extraData;

	};

}


#endif
