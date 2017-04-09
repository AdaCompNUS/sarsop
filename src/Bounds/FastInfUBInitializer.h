/** 
* Part of the this code is derived from ZMDP: http://www.cs.cmu.edu/~trey/zmdp/
* ZMDP is released under Apache License 2.0
* The rest of the code is released under GPL v2 
*/


#ifndef INCFastInfUBInitializer_h
#define INCFastInfUBInitializer_h

#include "MathLib.h"
#include "MOMDP.h"
#include "FacmodelStructs.h"
#include "BeliefValuePairPool.h"
#include "BeliefValuePairPoolSet.h"
#include "FullObsUBInitializer.h" // SYL260809 added

#define PRUNE_EPS (1e-10)
#define MDP_RESIDUAL (1e-10)

namespace momdp {

	struct FastInfUBInitializer 
	{
		SharedPointer<MOMDP> pomdp;
		BeliefValuePairPoolSet* bound;
		//UpperBoundBVpair* upperBoundBVpair;

		std::vector<alpha_vector> alphas;	// am assuming that we dont really need more than one set (for all state indexes) of alphas

		// SYL260809 commented out - it's not needed now that only one FullObsUBInitializer object is created
		//std::vector<alpha_vector> alphasByState;

		FastInfUBInitializer(SharedPointer<MOMDP> problem, BeliefValuePairPoolSet* _bound);
		FastInfUBInitializer(SharedPointer<MOMDP> problem);
		void initialize(double targetPrecision);
		void getFIBsolution(double targetPrecision);

// data and methods for factored version
		vector<vector<alpha_vector> > actionAlphaByState;


	protected:
		void initMDP(double targetPrecision, FullObsUBInitializer& m);  // SYL260809 prevly: void initMDP(double targetPrecision);
		void initFIB(double targetPrecision, bool getFIBvectors);

		void initMDP_unfac(double targetPrecision);
		void initFIB_unfac(double targetPrecision, bool getFIBvectors);


	};

}; // namespace zmdp

#endif /* INCFastInfUBInitializer_h */

