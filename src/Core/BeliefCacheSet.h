#ifndef BeliefCacheSet_H
#define BeliefCacheSet_H

#include "Const.h"
#include "BeliefWithState.h"
#include "BeliefCacheRow.h"

using namespace momdp;
class BeliefCacheSet
{
public:
	BeliefCacheSet(void);
	virtual ~BeliefCacheSet(void);

	int currentRowCount;
	
	BeliefCacheRow* getRow(cacherow_stval)
	{
	}

	bool hasBelief( BeliefWithState& bel)
	{
	}

	cacherow_stval addBeliefRow( BeliefWithState& bel)
	{
	}
	cacherow_stval addBeliefRowWithoutCheck( BeliefWithState& bel)
	{
	}
	cacherow_stval getBeliefRowIndex( BeliefWithState& bel)
	{
	}

	int size()
	{
		return 0;
	}
};

#endif
