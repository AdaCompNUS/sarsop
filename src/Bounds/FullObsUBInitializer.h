/** 
* Part of the this code is derived from ZMDP: http://www.cs.cmu.edu/~trey/zmdp/
* ZMDP is released under Apache License 2.0
* The rest of the code is released under GPL v2 
*/


#ifndef INCFullObsUBInitializer_h
#define INCFullObsUBInitializer_h

#include "MOMDP.h"

#define MDP_MAX_ITERS (1000000)

namespace momdp {

	class FullObsUBInitializer {
	public:
		SharedPointer<MOMDP> pomdp;	

  // data and methods for factored version
  std::vector<DenseVector> alphaByState;
		void nextAlphaAction(std::vector<DenseVector>& resultByState, int a);
		double valueIterationOneStep(void);
		void valueIteration(SharedPointer<MOMDP> _pomdp, double eps);

  // data and methods for unfactored version
		DenseVector alpha;
		void nextAlphaAction_unfac(DenseVector& result, int a);
		double valueIterationOneStep_unfac(void);
		void valueIteration_unfac(SharedPointer<MOMDP> _pomdp, double eps);


		// FOR QMDP
// data and methods for factored version
		vector<vector<alpha_vector> > actionAlphaByState;
		void QMDPSolution(SharedPointer<MOMDP> _pomdp, double eps);
		// these are not really needed - it is for computing a fixed action policy, starting with vector of all zero values, instead of the usual best-worst reward value.
		void QNextAlphaAction(std::vector<DenseVector>& resultByState, int a);
		double QValueIterationOneStep(void);
		void QValueIteration(SharedPointer<MOMDP> _pomdp, double eps);

// data and methods for factored version
		vector<alpha_vector> actionAlphas;
		void QMDPSolution_unfac(SharedPointer<MOMDP> _pomdp, double eps);

		// these are not really needed - it is for computing a fixed action policy, starting with vector of all zero values, instead of the usual best-worst reward value.
		void QNextAlphaAction_unfac(DenseVector& result, int a) ;
		double QValueIterationOneStep_unfac(void);
		void QValueIteration_unfac(SharedPointer<MOMDP> _pomdp, double eps);

		// Utils function
		void FacPostProcessing(vector<alpha_vector>& alphasByState);
		void UnfacPostProcessing(DenseVector& calpha, vector<alpha_vector>& alphasByState);

	};

}; // namespace zmdp

#endif // INCFullObsUBInitializer_h
