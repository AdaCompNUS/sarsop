/** 
* The code is released under GPL v2 
*/

#ifndef BinManager_H
#define BinManager_H

#include <iostream>
#include <string>
#include <vector>
#include "BeliefCache.h"
#include "BeliefValuePairPool.h"

using namespace std;

namespace momdp{
	class BinManagerDataTuple
	{
	public:
		bool binned;
	};

	class BinManager{
	public:
		//Constructor
		BinManager(BeliefValuePairPool * _initialUB, BeliefCache* _beliefCache, state_val _sval);
		void updateNode(int row);
		void printBinCount();
		double getBinValue(int row);

		IndexedTuple<BinManagerDataTuple> binManagerDataTable;
	private:
		
		BeliefValuePairPool* initialUB;
		BeliefCache* beliefCache; 
		double highest;
		double lowest;
		state_val sval;		// the value of observed state variable that this bounds corresponds to

		void initializeNode(BeliefTreeNode & node);
		void updateBin(BeliefTreeNode & node);

		// implementing dynamic bin levels
		const static int initial_bin_size = 5;
		const static int bin_growth_factor = 2;
		void initializeDynamicBins();
		int bin_level_count;
		map <int, //level #
			map <string,//ub_interval or entropy_interval or counter
			double//value of interval
			> 
		> 
		binLevels_intervals;

		map <int, //level #
			map <int, //node #
			map <string, //ub_index, entropy_index, prev_error
			double//value
			> 
			> 
		> 
		binLevels_nodes;

		map <int, //level #
			map <string, //bins, binCount, binError
			map <string,//ub-entropy access
			double//value inside bin
			> 
			> 
		> 
		binLevels;

		map <int, //node #
			double//value of previous lower bound
		>
		previous_lowerbound;
	};
}; // namespace momdp

#endif
