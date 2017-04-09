/** 
 * The code is released under GPL v2 
 */

#include "BinManager.h"
#include <cmath>
#include <map>

namespace momdp
{
	//constructor
	BinManager::BinManager(BeliefValuePairPool * _initialUB, BeliefCache* _beliefCache, state_val _sval)
	{
		initialUB = _initialUB;
		beliefCache = _beliefCache;
		sval = _sval;
		//beliefCache = &(BeliefCache::getCache ());
		initializeDynamicBins();
	}

	void BinManager::updateNode(int row)
	{
		BeliefTreeNode & node = *(beliefCache->getRow(row)->REACHABLE);
		if (binManagerDataTable.get(row).binned == false)
		{
			binManagerDataTable.set(row).binned = true;
			initializeNode(node);
		}
		else
		{			
			updateBin(node);
		}
	}

	void BinManager::initializeNode(BeliefTreeNode & node)
	{
		double lbVal = beliefCache->getRow(node.cacheIndex.row)-> LB ;
		double ubValue = initialUB->getValue(node.s->bvec);
		double entropy = 0;
		FOREACH_NOCONST (SparseVector_Entry, di, node.s->bvec->data) 
		{
			entropy += di->value * ( log(di->value) / log(2.0) );
		}
		entropy = (-1) * entropy;//negate summation

		int i;//start dynamic bins
		for (i=1;i<=bin_level_count;i++)
		{
			//upper bound
			binLevels_nodes[i][node.cacheIndex.row]["ub_index"] = (int) ((ubValue - lowest) / binLevels_intervals[i]["ub_interval"] );

			if ( binLevels_nodes[i][node.cacheIndex.row]["ub_index"] > 
				(initial_bin_size * pow((float)bin_growth_factor,(float)(i-1))) - 1 )
				binLevels_nodes[i][node.cacheIndex.row]["ub_index"] = 
				(initial_bin_size * pow((float)bin_growth_factor,(float)(i-1))) - 1;
			else
			if ( binLevels_nodes[i][node.cacheIndex.row]["ub_index"] < 0 )
				binLevels_nodes[i][node.cacheIndex.row]["ub_index"] = 0;

			//entropy
			binLevels_nodes[i][node.cacheIndex.row]["entropy_index"] = (int) (entropy / binLevels_intervals[i]["entropy_interval"]);
			if ( binLevels_nodes[i][node.cacheIndex.row]["entropy_index"] >
				(initial_bin_size * pow((float)bin_growth_factor,(float)(i-1))) - 1 )
				binLevels_nodes[i][node.cacheIndex.row]["entropy_index"] = 
				(initial_bin_size * pow((float)bin_growth_factor,(float)(i-1))) - 1;
			else
			if ( binLevels_nodes[i][node.cacheIndex.row]["entropy_index"] < 0 )
				binLevels_nodes[i][node.cacheIndex.row]["entropy_index"] = 0;
			
			char access[100];//ub-entropy
			sprintf( access, "%d-%d", (int)binLevels_nodes[i][node.cacheIndex.row]["ub_index"], (int)binLevels_nodes[i][node.cacheIndex.row]["entropy_index"]);
		
			//prediction error
			if ( binLevels[i]["binCount"][access] == 0 )
			{
				double err = beliefCache->getRow(node.cacheIndex.row)->UB - lbVal;
				binLevels_nodes[i][node.cacheIndex.row]["prev_error"] = err * err;
				binLevels[i]["binError"][access] = binLevels_nodes[i][node.cacheIndex.row]["prev_error"];
			}
			else
			{
				double err = binLevels[i]["bins"][access] - lbVal;
				binLevels_nodes[i][node.cacheIndex.row]["prev_error"] = err * err;
				binLevels[i]["binError"][access] += binLevels_nodes[i][node.cacheIndex.row]["prev_error"];
			}

			//adding new point and calculating new average
			binLevels[i]["bins"][access] = ((double)(binLevels[i]["bins"][access] * binLevels[i]["binCount"][access] + lbVal)) / ((double)(binLevels[i]["binCount"][access] + 1));

			binLevels[i]["binCount"][access] ++;
		}//end dynamic bins	
		previous_lowerbound[node.cacheIndex.row] = lbVal;
	}

	void BinManager::updateBin(BeliefTreeNode & node)
	{
		double lbVal = beliefCache->getRow(node.cacheIndex.row)->LB;
		int i;//start dynamic bins
		for (i=1;i<=bin_level_count;i++)
		{
			char access[100];//ub-entropy
			sprintf( access, "%d-%d", (int)binLevels_nodes[i][node.cacheIndex.row]["ub_index"], (int)binLevels_nodes[i][node.cacheIndex.row]["entropy_index"]);

			//prediction error
			if ( binLevels[i]["binCount"][access] == 1 )
			{
				double err = beliefCache->getRow(node.cacheIndex.row) ->UB - lbVal;
				binLevels_nodes[i][node.cacheIndex.row]["prev_error"] = err * err;
				binLevels[i]["binError"][access] = binLevels_nodes[i][node.cacheIndex.row]["prev_error"];
			}
			else
			{
				double err = binLevels[i]["bins"][access]- lbVal;
				binLevels[i]["binError"][access] -= binLevels_nodes[i][node.cacheIndex.row]["prev_error"];
				binLevels_nodes[i][node.cacheIndex.row]["prev_error"] = err * err;
				binLevels[i]["binError"][access] += binLevels_nodes[i][node.cacheIndex.row]["prev_error"];
			}


			//updating point average value
			binLevels[i]["bins"][access] = ((double)(binLevels[i]["bins"][access] * binLevels[i]["binCount"][access] + lbVal - previous_lowerbound[node.cacheIndex.row])) / ((double)binLevels[i]["binCount"][access]);
		}//end dynamic bins
		previous_lowerbound[node.cacheIndex.row] = lbVal;
	}

	void BinManager::printBinCount()
	{
		int x, y, i;
		for (i=1;i<=bin_level_count;i++)
		{
			cout << endl << "entropy --------------------------------------->" << endl;
			for (x=0;x<(initial_bin_size * pow((float)bin_growth_factor,(float)(i-1)));x++)
			{
				for(y=0;y<(initial_bin_size * pow((float)bin_growth_factor,(float)(i-1)));y++)
				{
					char access[100];//ub-entropy
					sprintf( access, "%d-%d", x, y);
					cout << ((int)binLevels[i]["binCount"][access]) << "\t";
					
				}
				cout << endl;
			}
			cout << endl << "level " << i << ": " << binLevels_intervals[i]["counter"] << endl;
		}
	}

	double BinManager::getBinValue(int row)
	{
		//updateNode(row);
		double lbVal = beliefCache->getRow(row)->LB;
		double ubVal = beliefCache->getRow(row)->UB;
		char access[100];//ub-entropy
		sprintf( access, "%d-%d", (int)binLevels_nodes[1][row]["ub_index"], (int)binLevels_nodes[1][row]["entropy_index"]);

		if ( ((int)binLevels[1]["binCount"][access]) == 1 )//new node
		{
			//encourage exploration by returning upper bound
			return ubVal;
		}
		else
		{
			int i, best_level;
			double smallest_error;
			for (i=1;i<=bin_level_count;i++)//bin selection criteria
			{
				sprintf( access, "%d-%d", (int)binLevels_nodes[i][row]["ub_index"], (int)binLevels_nodes[i][row]["entropy_index"]);
				if ( i == 1 ) 
				{
					best_level = i;
					smallest_error = binLevels[i]["binError"][access];
				}

				if ( binLevels[i]["binError"][access] + 1e-10 < smallest_error )
				{
					best_level = i;
					smallest_error = binLevels[i]["binError"][access];
				}
			}

			sprintf( access, "%d-%d", (int)binLevels_nodes[best_level][row]["ub_index"], (int)binLevels_nodes[best_level][row]["entropy_index"]);

			binLevels_intervals[best_level]["counter"] ++;


			if ( binLevels[best_level]["bins"][access] > ubVal + 1e-10 ) return ubVal;
			else
			if ( binLevels[best_level]["bins"][access] + 1e-10 < lbVal ) return lbVal;
			else
			return binLevels[best_level]["bins"][access];
		}
	}

	void BinManager::initializeDynamicBins()
	{
		bin_level_count = 2;

		//initial upper bound
		highest = -99e10;
		lowest = 99e10;
		int numStates = initialUB->problem->getBeliefSize();
		FOR (i, numStates) {
			double value = initialUB->cornerPoints(i);
			if ( value > highest ) highest = value;
			if ( value < lowest ) lowest = value;
		}

		//entropy
		double maxEntropy = 0;
		maxEntropy = ((double)(1.0/numStates)) * ( log( ((double)(1.0/numStates)) ) / log(2.0) ) * (-1) * numStates;
		
		int i;
		for (i=1;i<=bin_level_count;i++)
		{
			//binLevels_intervals;
			map <string,//ub_interval or entropy_interval
				double//value of interval
			> intervals;
			intervals["ub_interval"] = (highest - lowest) / (initial_bin_size * pow((float)bin_growth_factor,(float)(i-1)));
			intervals["entropy_interval"] = maxEntropy / (initial_bin_size * pow((float)bin_growth_factor,(float)(i-1)));
			binLevels_intervals[i] = intervals;


			//binLevels_nodes;
			map <int, //node #
				map <string, //ub_index, entropy_index, prev_error
					double//value
				> 
			> nodes;
			binLevels_nodes[i] = nodes;
			
			
			//binLevels;
			map <string, //bins, binCount, binError
				map <string,//ub-entropy (bin id)
					double//value inside bin
				> 
			> level;

			map <string,//ub-entropy (bin id)
				double//value inside bin
			> bins;

			map <string,//ub-entropy (bin id)
				double//value inside bin
			> binCount;

			map <string,//ub-entropy (bin id)
				double//value inside bin
			> binError;

			level["bins"] = bins;
			level["binCount"] = binCount;
			level["binError"] = binError;
			binLevels[i] = level;
		}	
	}
};
