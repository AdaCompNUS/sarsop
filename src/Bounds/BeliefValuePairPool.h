#ifndef BeliefValuePairPool_H
#define BeliefValuePairPool_H

#include "Bound.h"
#include "Backup.h"
#include "Tuple.h"
#include "IndexedTuple.h"
#include "BeliefValuePair.h"
#include "PruneBeliefValuePair.h"
#include <exception>
#include <list>
#include <vector>
#include <stdexcept>


using namespace std;
using namespace momdp;
namespace momdp 
{

#define CORNER_EPS (1e-6)
#define MIN_RATIO_EPS (1e-10)

	class BeliefValuePairPoolDataTuple :public Tuple
	{
	public:
		int UB_ACTION; /*the max ub action*/
	};


	class BeliefValuePairPool :	public Bound<BeliefValuePair>
	{
	public:
		BeliefCache *beliefCache;
		void setBeliefCache(BeliefCache *p)
		{
			beliefCache = p;
		}
		IndexedTuple<BeliefValuePairPoolDataTuple> *dataTable;
		void setDataTable(IndexedTuple<BeliefValuePairPoolDataTuple> *p)
		{
			dataTable = p;
		}

		SharedPointer<MOMDP> problem;
		void setProblem(SharedPointer<MOMDP> p)
		{
			problem = p;
		}
		void setSolver(PointBasedAlgorithm *p)
		{
			//solver = p;
		}

		BeliefValuePairPool(Backup<BeliefValuePair> *_backupEngine)
		{
			this->setBackupEngine(_backupEngine);
			cornerPointsVersion = 0;
		}
		PruneBeliefValuePair* pruneEngine;

		virtual ~BeliefValuePairPool(void)
		{
		}


		virtual REAL_VALUE getValue(SharedPointer<Belief>& belief);
		double getValue_NoInterpolation(const belief_vector& b);

		virtual SharedPointer<BeliefValuePair> backup(BeliefTreeNode * node)
		{
			SharedPointer<BeliefValuePair> result = backupEngine->backup(node);
			for(size_t i = 0 ; i < onBackup.size(); i++)
			{
				(*onBackup[i])(solver, node, result);
			}
			throw runtime_error("Not finished...");
			return result;
		}


		unsigned int cornerPointsVersion;
		DenseVector cornerPoints;


		list<SharedPointer<BeliefValuePair> > points;
		// update helper functions
		SharedPointer<BeliefValuePair> addPoint(SharedPointer<belief_vector>&  b, double val);
		int whichCornerPoint(const SharedPointer<belief_vector>&  b) const;

	private:	
		void printCorners() const;


	};


}

#endif 

