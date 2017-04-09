/** 
 * This code is released under GPL v2
 */

#include <sstream>
#include <iostream>

#include "UniqueBeliefHeap.h"
#include "BeliefException.h"
#include "GlobalResource.h"


using namespace std;
#ifdef _MSC_VER
using namespace stdext;
#endif

#define HashResolveRes 1.0e-7

namespace momdp
{
	int UniqueBeliefHeap::addBeliefRowPair(SharedPointer<belief_vector>& bel, int row)
	{
		//  string newHash = bel.hashCode();
		// cout << "addBeliefRowPair: bel size: " << bel.size() << endl;


		string newHash = bel->md5HashValue();
		//cout << "Hash: " << newHash << endl;


		//if(!isStorageMaterial(bel))
		//{
		//	cout << "Trying to add a duplicate hash key " << newHash << endl;
		//	return -1;
		//}

		map<string, vector<BeliefVector_RowIndexPair> > ::iterator iter = uniqueBelHeap.find(newHash);
		
		if(iter == uniqueBelHeap.end())
		{
			vector<BeliefVector_RowIndexPair> *vecBvrip = new vector<BeliefVector_RowIndexPair>();
			
			BeliefVector_RowIndexPair *bvrip = new BeliefVector_RowIndexPair(bel, row);
			vecBvrip->push_back(*bvrip);

			uniqueBelHeap[newHash] = *vecBvrip;
		}
		else
		{
			BeliefVector_RowIndexPair *bvrip = new BeliefVector_RowIndexPair(bel, row);
			iter->second.push_back(*bvrip);
			//uniqueBelHeap[newHash].push_back();
		}

		

		//cout << "-------------- " << uniqueBelHeap[newHash].size() << " -----------------------------" << endl;
		//printHashMap();

		return row;
	}


	bool UniqueBeliefHeap::hasBelief(SharedPointer<belief_vector>& bel)
	{
		int rowIndex = getBeliefRowIndex(bel);
		return (rowIndex >= 0);
	}

	int UniqueBeliefHeap::getBeliefRowIndex(SharedPointer<belief_vector>& bel)const 
	{
		string newHash = bel->md5HashValue();
		GlobalResource::getInstance()->hashRequest ++;
		map<string, vector<BeliefVector_RowIndexPair> > ::const_iterator iter = uniqueBelHeap.find(newHash);
		if(iter != uniqueBelHeap.end())
		{
			GlobalResource::getInstance()->hashSame ++;
			// same hash, should go and check each element inside
			vector<BeliefVector_RowIndexPair> vecBvrip = iter->second;
			for(int i = 0 ; i < vecBvrip.size(); i++)
			{
				BeliefVector_RowIndexPair bvrip = vecBvrip[i];

				// Resolve by total difference, currently disabled
				/* 
				double	diff = bvrip.vector->totalDifference(bel);
				if (diff < 0.0001)
				{
					// difference small enough to be considered same vector;
					//cout << "Not Collision : diff: " << diff << endl;
					return bvrip.row;
				}
				*/
				
				// Consider different if single differs
				if(!bvrip.vector->isDifferentByAtLeastSingleEntry(*bel, HashResolveRes))
				{
					return bvrip.row;
				}
			}
			//cout << "Collision" << endl;
			GlobalResource::getInstance()->hashCollision ++;
			return -1;
		}
		else
		{
			return -1;
		}
	}

	
	void UniqueBeliefHeap::printHashMap()const 
	{
		map<string, vector<BeliefVector_RowIndexPair> > ::const_iterator iter = uniqueBelHeap.begin();
		for(; iter != uniqueBelHeap.end();iter++)
		{
			cout << "Key: " << iter->first << " Length: " << iter->second.size() << endl;
		}

	}
	bool UniqueBeliefHeap::isStorageMaterial(SharedPointer<belief_vector>& bel) {
		return !hasBelief(bel);
	}

	UniqueBeliefHeap::~UniqueBeliefHeap(){
		//  uniqueBelHeap.clear();
	}
}
