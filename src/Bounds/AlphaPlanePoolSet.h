#ifndef AlphaPlanePoolSet_H
#define AlphaPlanePoolSet_H

#include <exception>
#include <list>
#include <vector>
#include <stdexcept>
#include "Bound.h"
#include "Backup.h"
#include "AlphaPlane.h"
#include "AlphaPlanePool.h"
#include "MOMDP.h"

#define sparse_vector

using namespace std;
using namespace momdp;
namespace momdp 
{
	class AlphaPlanePoolSet :	public Bound<AlphaPlane>
	{
	private:
	public:
		vector<AlphaPlanePool*> set;
		SharedPointer<MOMDP> problem;
		
		void setProblem(SharedPointer<MOMDP> p)
		{
			problem = p;
		}
		void setSolver(PointBasedAlgorithm *p)
		{
			solver = p;
		}

		vector<BeliefCache *> beliefCacheSet;
		void setBeliefCache(vector<BeliefCache *> p)
		{
			beliefCacheSet = p;
			for(int i = 0 ; i < set.size(); i ++)
			{
				set[i]->setBeliefCache(p[i]);
			}
		}
		vector<IndexedTuple<AlphaPlanePoolDataTuple> *> dataTableSet;
		void setDataTable(vector<IndexedTuple<AlphaPlanePoolDataTuple> *> p)
		{
			dataTableSet = p;
			for(int i = 0 ; i < set.size(); i ++)
			{
				set[i]->setDataTable(p[i]);
			}
		}
		
		AlphaPlanePoolSet(Backup<AlphaPlane> *_backupEngine)
		{
			this->setBackupEngine(_backupEngine);
		}
		void initialize()
		{
			set.resize(problem->XStates->size());
			for(States::iterator iter = problem->XStates->begin(); iter != problem->XStates->end(); iter ++ )
			{
				AlphaPlanePool *bound = new AlphaPlanePool(NULL);
				bound->setProblem(problem);
				bound->setSolver(solver);
				bound->pruneEngine = new PruneAlphaPlane();
				bound->pruneEngine->setup(problem, bound);
				set[iter.index()] = bound;
			}
		}
		virtual ~AlphaPlanePoolSet(void)
		{
		}
		virtual void appendOnBackupHandler(BackupCallback _onBackup)
		{
			for(int i = 0 ; i < set.size(); i ++)
			{
				set[i]->appendOnBackupHandler(_onBackup);
			}
		}
		virtual void removeOnBackupHandler(BackupCallback _onBackup)
		{
			for(int i = 0 ; i < set.size(); i ++)
			{
				set[i]->removeOnBackupHandler(_onBackup);
			}
		}

		virtual REAL_VALUE getValue(SharedPointer<BeliefWithState>& belief);
		virtual SharedPointer<AlphaPlane> backup(BeliefTreeNode * node);
		virtual SharedPointer<AlphaPlane> getValueAlpha(SharedPointer<BeliefWithState>& belief);


		SharedPointer<AlphaPlane> getBestAlphaPlane(BeliefWithState& b);
		SharedPointer<AlphaPlane> getBestAlphaPlane(BeliefTreeNode& cn);
		SharedPointer<AlphaPlane> getBestAlphaPlane1(BeliefWithState& beliefandState);
		
		void writeToFile(const std::string& outFileName, string problemName) ; //write in new XML format
//		void writeToFile(const std::string& outFileName) ; //write in out IV format
	};
}

#endif 

