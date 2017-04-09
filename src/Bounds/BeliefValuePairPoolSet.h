#ifndef BeliefValuePairPoolSet_H
#define BeliefValuePairPoolSet_H

#include "Bound.h"
#include "Backup.h"
#include "MOMDP.h"
#include "BeliefValuePair.h"
#include "BeliefValuePairPool.h"
#include "PruneBeliefValuePair.h"
#include <exception>
#include <vector>
using namespace std;
using namespace momdp;
namespace momdp 
{


	class BeliefValuePairPoolSet :	public Bound<BeliefValuePair>
	{
	private:
		
	public:
		vector<BeliefValuePairPool*> set;
		vector<BeliefCache *> beliefCacheSet;
		void setBeliefCache(vector<BeliefCache *> p)
		{
			beliefCacheSet = p;
			for(size_t i = 0 ; i < set.size(); i ++)
			{
				set[i]->setBeliefCache(p[i]);
			}
		}

		vector<IndexedTuple<BeliefValuePairPoolDataTuple> *> dataTableSet;
		void setDataTable(vector<IndexedTuple<BeliefValuePairPoolDataTuple> *> p)
		{
			dataTableSet = p;
			for(size_t i = 0 ; i < set.size(); i ++)
			{
				set[i]->setDataTable(p[i]);
			}
		}

		SharedPointer<MOMDP> problem;
		void setProblem(SharedPointer<MOMDP> p)
		{
			problem = p;
		}
		void setSolver(PointBasedAlgorithm *p)
		{
			solver = p;
		}

		BeliefValuePairPoolSet(Backup<BeliefValuePair> *_backupEngine)
		{
			this->setBackupEngine(_backupEngine);
		}

		void initialize()
		{
			set.resize(problem->XStates->size());
			for(States::iterator iter = problem->XStates->begin(); iter != problem->XStates->end(); iter ++ )
			{
				// Should not use BeliefValuePairPool's backup
				BeliefValuePairPool *bound = new BeliefValuePairPool(NULL);
				bound->setProblem(problem);
				bound->setSolver(solver);
				set[iter.index()] = bound;
				bound->pruneEngine = new PruneBeliefValuePair();
				bound->pruneEngine->setup(bound);
			}
		}

		SharedPointer<BeliefValuePair> addPoint(SharedPointer<BeliefWithState> beliefandState, double val) 
		{
			state_val sval = beliefandState->sval;
			SharedPointer<belief_vector>  b = beliefandState->bvec;
			return set[sval]->addPoint(b, val);
		}

		virtual void appendOnBackupHandler(BackupCallback _onBackup)
		{
			for(size_t i = 0 ; i < set.size(); i ++)
			{
				set[i]->appendOnBackupHandler(_onBackup);
			}
		}
		virtual void removeOnBackupHandler(BackupCallback _onBackup)
		{
			for(size_t i = 0 ; i < set.size(); i ++)
			{
				set[i]->removeOnBackupHandler(_onBackup);
			}
		}
		virtual ~BeliefValuePairPoolSet(void)
		{
		}

		virtual REAL_VALUE getValue(SharedPointer<BeliefWithState>& belief);

		virtual SharedPointer<BeliefValuePair> backup(BeliefTreeNode * node)
		{
			SharedPointer<BeliefValuePair> result = backupEngine->backup(node);
			for(size_t i = 0 ; i < onBackup.size(); i++)
			{
				(*onBackup[i])(solver, node, result);
			}
			return result;
		}

	};


}

#endif 

