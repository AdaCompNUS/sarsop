#ifndef PointBasedAlgorithm_H
#define PointBasedAlgorithm_H

#include <vector>
#include "Const.h"
#include "solverUtils.h"
#include "MOMDP.h"

using namespace std;
using namespace momdp;
namespace momdp 
{
	class BeliefForest;
	class BeliefCache;
	class Sample;

	class PointBasedAlgorithm
	{
	public:
		SolverParams * solverParams;
		SharedPointer<MOMDP> problem;
		BeliefForest* beliefForest;
		vector <BeliefCache *> beliefCacheSet;
		// Sample Related
		Sample* sampleEngine;

		int numBackups;

		PointBasedAlgorithm(void)
		{
		}
		virtual ~PointBasedAlgorithm(void)
		{
		}

		virtual void writePolicy(string fileName, string problemName) = 0;
		virtual void solve(SharedPointer<MOMDP> problem) = 0;
		
		// TODO use factory method to dynamically produce solver specific data tuple
		virtual void* getSolverData()
		{
			return NULL;
		}
	};

}


#endif
