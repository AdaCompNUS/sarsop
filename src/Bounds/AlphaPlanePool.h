#ifndef AlphaPlanePool_H
#define AlphaPlanePool_H

#include "Bound.h"
#include "Backup.h"
#include "AlphaPlane.h"
#include "IndexedTuple.h"
#include "PruneAlphaPlane.h"

#include <exception>
#include <list>
#include <vector>
#include <stdexcept>
using namespace std;
using namespace momdp;
namespace momdp 
{
	class AlphaPlanePoolDataTuple :public Tuple
	{
	public:
		int ALPHA_TIME_STAMP;
		list<SharedPointer<AlphaPlane> >* ALPHA_PLANES; /*alpha planes which dominate at this SharedPointer<Belief> */
	};


	class AlphaPlanePool :	public Bound<AlphaPlane>
	{
	public:
		AlphaPlanePool(Backup<AlphaPlane> *_backupEngine)
		{
			this->setBackupEngine(_backupEngine);
		}
		PruneAlphaPlane* pruneEngine;

		virtual ~AlphaPlanePool(void)
		{
		}



		virtual SharedPointer<AlphaPlane> backup(BeliefTreeNode * node)
		{
			SharedPointer<AlphaPlane> result = backupEngine->backup(node);
			for(size_t i = 0 ; i < onBackup.size(); i++)
			{
				(*onBackup[i])(solver, node, result);
			}
			throw runtime_error("Not finished...");
			return result;
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
		BeliefCache *beliefCache;
		void setBeliefCache(BeliefCache *p)
		{
			beliefCache = p;
		}


		IndexedTuple<AlphaPlanePoolDataTuple> *dataTable;
		void setDataTable(IndexedTuple<AlphaPlanePoolDataTuple> *p)
		{
			dataTable = p;
		}

		SharedPointer<AlphaPlane> getBestAlphaPlane(SharedPointer<belief_vector>& b);
		SharedPointer<AlphaPlane> getBestAlphaPlane(BeliefTreeNode& cn);	
		SharedPointer<AlphaPlane> getBestAlphaPlane1(SharedPointer<belief_vector>& b);
		SharedPointer<AlphaPlane> getBestAlphaPlane( SharedPointer<belief_vector>& b, int index );  //  belief, belief index
	
		virtual double getValue(SharedPointer<belief_vector>& belief);
		// TODO:: Phase out this
		virtual SharedPointer<AlphaPlane> getValueAlpha(SharedPointer<Belief>& belief);

		virtual double getValue(SharedPointer<belief_vector>& belief, SharedPointer<AlphaPlane>* bestAlpha);

		list<SharedPointer<AlphaPlane> > planes;
		void addAlphaPlane(SharedPointer<AlphaPlane> plane);
	private:
		
		
	};
}

#endif 

