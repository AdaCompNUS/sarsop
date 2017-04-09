/** 
 * The code is released under GPL v2 
 */

#include "BinManagerSet.h"
#include "BinManager.h"
//#include <cmath>
//#include <map>

namespace momdp
{
	//constructor
	BinManagerSet::BinManagerSet(BeliefValuePairPoolSet * _bounds)
	{
		bounds = _bounds;
		size_t xStateNum = bounds->problem->XStates->size();
		binManagerSet.resize(xStateNum);

		for (size_t state_indx = 0; state_indx <xStateNum; state_indx++) 
		{
			binManagerSet[state_indx] = new BinManager(bounds->set[state_indx], bounds->set[state_indx]->beliefCache, state_indx); 
		}
	}

	void BinManagerSet::updateNode(cacherow_stval cacheIndexRow)
	{
		binManagerSet[cacheIndexRow.sval]->updateNode(cacheIndexRow.row);

	}


	void BinManagerSet::printBinCount()
	{
		FOR (stateidx, bounds->problem->XStates->size()) {
			binManagerSet[stateidx]->printBinCount();

		}
 	}


	double BinManagerSet::getBinValue(cacherow_stval cacheIndexRow)
	{

		return binManagerSet[cacheIndexRow.sval]->getBinValue(cacheIndexRow.row);
	}

};
