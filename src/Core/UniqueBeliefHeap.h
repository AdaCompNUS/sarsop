/** 
* This code is released under GPL v2 
* @file UniqueBeliefHeap.h
* @brief This file implements a belief heap having one to one relationship between
* a belief and a row index. There is not duplication of beliefs....
* @author Amit Jain
* @date 2007-03-30
*/

#ifndef UNIQUEBELIEFHEAP3_H
#define UNIQUEBELIEFHEAP3_H

#include <map>
#include <string>
#include <vector>
#include "MathLib.h"
#include "MObject.h"
using namespace std;

namespace momdp{

	struct BeliefVector_RowIndexPair
	{
		BeliefVector_RowIndexPair(SharedPointer<belief_vector>& bel, int row)
		{
			vector = bel;
			this->row = row;
		}

		SharedPointer<belief_vector> vector;
		int row;
	};
	class UniqueBeliefHeap
	{
	private:
		map<string, vector<BeliefVector_RowIndexPair> > uniqueBelHeap;

		bool hasBelief(SharedPointer<belief_vector>& bel);
	public:
		/** 
		* @brief This for adding a belief and row pair
		* 
		* @param bel belief to be added
		* @param row row index
		*/
		int addBeliefRowPair(SharedPointer<belief_vector>& bel, int row);
		//void addBeliefRowPair(const Belief& bel, int row);

		/** 
		* @brief get the corresponding row, -1 is returned if the thing is not found
		* 
		* @param bel belief being searched
		* 
		* @return row index in the belief cache
		*/
		int getBeliefRowIndex(SharedPointer<belief_vector>& bel) const;
		//int getBeliefRowIndex(const Belief& bel)const;

		/** 
		* @brief This is for checking whether the current belief in question can be added to
		* the heap / cache or not
		* 
		* @param bel the belief in question
		* 
		* @return can it be added
		*/
		bool isStorageMaterial(SharedPointer<belief_vector>& bel);
		//bool isStorageMaterial(const Belief& bel)const;

		void printHashMap()const ;

		virtual ~UniqueBeliefHeap();
	};
}
#endif
