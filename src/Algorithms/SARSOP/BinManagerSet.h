/** 
 * The code is released under GPL v2 
 */

#ifndef BinManagerSet_H
#define BinManagerSet_H

#include <iostream>
#include <string>
#include <vector>
//#include "UpperBoundBVpair.h"
//#include "BeliefCache.h"
#include "FacmodelStructs.h"
#include "BinManager.h"
#include "BeliefValuePairPoolSet.h"


using namespace std;

namespace momdp{

	class BinManagerSet
	{
		public:
			//Constructor
			BinManagerSet(BeliefValuePairPoolSet * _bounds);
			void updateNode(cacherow_stval cacheIndexRow); // void updateNode(int row);
			void printBinCount();
			double getBinValue(cacherow_stval cacheIndexRow); // double getBinValue(int row);

			vector <BinManager*>  binManagerSet;
		private:
			BeliefValuePairPoolSet * bounds;
			
			//UpperBoundBVpair* initialUB;
			//BeliefCache* beliefCache; 
			//double highest;
			//double lowest;

			//void initializeNode(MDPNode & node);
			//void updateBin(MDPNode & node);

			// implementing dynamic bin levels
			/* const static int initial_bin_size = 5;
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

			*/
	};
}; // namespace momdp

#endif
