/** 
 * Part of the this code is derived from ZMDP: http://www.cs.cmu.edu/~trey/zmdp/
 * ZMDP is released under Apache License 2.0
 * The rest of the code is released under GPL v2 
 */



#ifndef PruneBeliefValuePair_H
#define PruneBeliefValuePair_H


#include <iostream>
#include <string>
#include <vector>
#include <list>

#include "MOMDP.h"


#define PRUNE_PTS_INCREMENT (10)
#define PRUNE_PTS_FACTOR (1.1)


using namespace std;
using namespace momdp;

namespace momdp
{
    /**
     * class PruneBeliefValuePair
     */
    class BeliefValuePairPool;

    class PruneBeliefValuePair 
	{
    public:
	
	int lastPruneNumPts;
	BeliefValuePairPool* bvPairPool;
		
	PruneBeliefValuePair(){}
	
	void setup(	BeliefValuePairPool* _bvPairPool)
	{
	    lastPruneNumPts = 0;
		bvPairPool = _bvPairPool;
	}
	
	void prune(void);
	
	void prunePoints(void);
	
    protected:
	//for debugging purpose
	void printCorners()const;
	
	
    };
};
#endif // PRUNEBVPAIR_H

