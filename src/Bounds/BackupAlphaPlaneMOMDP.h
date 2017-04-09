#ifndef BackupAlphaPlaneMOMDP_H
#define BackupAlphaPlaneMOMDP_H

#include "Backup.h"
#include "AlphaPlane.h"
#include "AlphaPlanePool.h"
#include "AlphaPlanePoolSet.h"
#include "MOMDP.h"
#include "IndexedTuple.h"
using namespace momdp;

namespace momdp 
{
	class AlphaPlanePool;
	class BeliefCache;
	class PointBasedAlgorithm;

	class BackupAlphaPlaneMOMDP : public Backup<AlphaPlane>
	{
	public:
		BackupAlphaPlaneMOMDP(void);
		virtual ~BackupAlphaPlaneMOMDP(void);

		SharedPointer<MOMDP> problem;
		AlphaPlanePoolSet *boundSet;
		
		PointBasedAlgorithm* solver;
		//BeliefCache* beliefCache;
		//IndexedTuple<AlphaPlanePoolDataTuple> *dataTable;

		void setProblem(SharedPointer<MOMDP> p)
		{
			problem = p;
		}
		void setSolver(PointBasedAlgorithm *p)
		{
			solver = p;
		}

		virtual SharedPointer<AlphaPlane> backup(BeliefTreeNode * node);
		virtual	double getNewAlphaPlane(AlphaPlane& result, BeliefTreeNode& cn);
		virtual void getNewAlphaPlaneQ(AlphaPlane& result, const BeliefTreeNode& cn, int a);

	};
}
#endif

