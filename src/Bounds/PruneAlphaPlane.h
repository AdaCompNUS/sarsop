/** 
* Part of the this code is derived from ZMDP: http://www.cs.cmu.edu/~trey/zmdp/
* ZMDP is released under Apache License 2.0
* The rest of the code is released under GPL v2 
*/



#ifndef PruneAlphaPlane_H
#define PruneAlphaPlane_H

#include "MOMDP.h"
#include "AlphaPlane.h"
using namespace std;
using namespace momdp;
namespace momdp
{
	class AlphaPlanePool;
	class PruneAlphaPlane 
	{
	public:
		PruneAlphaPlane();

		SharedPointer<MOMDP> problem;
		AlphaPlanePool* alphaPlanePool;
		int lastPruneNumPlanes;

		double global_delta;

		double pruneTime;
		int numPrune;
		// int state;	//SYL COMMENTED OUT

		void setup(SharedPointer<MOMDP> _problem, AlphaPlanePool* _alphaPlanePool)
		{
			lastPruneNumPlanes = 0;

			problem = _problem;
			alphaPlanePool = _alphaPlanePool;
			global_delta = 0.1;

			pruneTime = 0.0;
			numPrune = 0;
			//state = 2;		//SYLMOD
		}
		////////////////////////////////////
		//pruning public methods
		////////////////////////////////////
		void prune(void);
		void prunePlanes(void);
		void pruneNotCertedAndNotUsed(void);
		void updateCertsAndUses(int timeStamp);

		//statistical methods accessing 'count' and 'used'
		int countUses(void);
		int countUsedPlanes(void);
		int countCerts(void);
		int countCertedPlanes(void);
		void resetUseds(void);//added!! rn 20060925

		void pruneDynamicDeltaVersion(int timeStamp, int&, int&);  // SYL MOD

		void Sanity();
		void setDelta(double newDelta);

	protected:
		/////////////////////////////////////
		//protected pruning related methods
		/////////////////////////////////////
		void updateCorners(SharedPointer<AlphaPlane> plane, std::vector<double, std::allocator<double> >* values, std::vector<SharedPointer<AlphaPlane>, std::allocator<SharedPointer<AlphaPlane> > >* indices);
		void updateCerts(SharedPointer<AlphaPlane> plane, double delta, int timeStamp);
		void updateMax(SharedPointer<AlphaPlane> plane, double delta, int timeStamp);
		void updateUsesByCorners(void);
		void updateCertsByDeltaDominance(double delta, int timeStamp);



		void resetAlphaPlaneStateMachine();
		void computePruneStats(int *oP, int *uP);
		//void updateDelta(int overPrune, int underPrune);  // SYL - not used
		//void updateDeltaVersion2(int overPrune, int underPrune);	// SYL - these three moved to Bounds
		//void increaseDelta();
		//void decreaseDelta();

		static bool isMax(SharedPointer<AlphaPlane> alphaPlane);

	};
};

#endif // PRUNEALPHA_H
