/** 
* Part of the this code is derived from ZMDP: http://www.cs.cmu.edu/~trey/zmdp/
* ZMDP is released under Apache License 2.0
* The rest of the code is released under GPL v2 
*/


#include "PruneBeliefValuePair.h"
#include "BeliefValuePair.h"
#include "BeliefValuePairPool.h"

using namespace std;
using namespace momdp;

namespace momdp{

	void PruneBeliefValuePair::prune(void) 
	{
		unsigned int nextPruneNumPts = max(lastPruneNumPts + PRUNE_PTS_INCREMENT, (int) (lastPruneNumPts * PRUNE_PTS_FACTOR));
		if (bvPairPool->points.size() > nextPruneNumPts) 
		{
			prunePoints();
		}
	}

	void PruneBeliefValuePair::prunePoints(void) 
	{
#if USE_DEBUG_PRINT
		int oldNum = bvPairPool->points.size();
#endif

		list< std::list<SharedPointer<BeliefValuePair> >::iterator > erase_ptrs;

		LISTFOREACH_NOCONST(SharedPointer<BeliefValuePair> , ptp,  bvPairPool->points) 
		{
			SharedPointer<BeliefValuePair> try_pair = *ptp;
			try_pair->disabled = true;
			// put pt back in if v is at least epsilon smaller than
			// upperBound(b) (smaller is better because it means the upper
			// bound is tighter)
			if (try_pair->v <= bvPairPool->getValue((try_pair->b)) - ZMDP_BOUNDS_PRUNE_EPS) 
			{
				try_pair->disabled = false;
			} 
			else
			{
				erase_ptrs.push_back(ptp);
			}
		}

		LISTFOREACH(list<SharedPointer<BeliefValuePair> >::iterator, erase_ptr,  erase_ptrs) 
		{
			list<SharedPointer<BeliefValuePair> >::iterator  x, xp1;
			x = xp1 = (*erase_ptr);
			xp1++;
			bvPairPool->points.erase( x, xp1 );
		}

#if USE_DEBUG_PRINT
		cout << "... pruned # points from " << oldNum << " down to " << points.size() << endl;
#endif
		lastPruneNumPts = bvPairPool->points.size();
	}





};

