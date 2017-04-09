/** 
 * Part of the this code is derived from ZMDP: http://www.cs.cmu.edu/~trey/zmdp/
 * ZMDP is released under Apache License 2.0
 * The rest of the code is released under GPL v2 
 */

#include "PruneAlphaPlane.h"
#include "AlphaPlanePool.h"
#include "GlobalResource.h"
#include "SARSOP.h"
#include <cfloat>


using namespace std;
using namespace momdp;
namespace momdp
{
	/*************************************************************
	PRUNING METHODS
	**************************************************************/

	PruneAlphaPlane::PruneAlphaPlane()
	{
		lastPruneNumPlanes = 0;
		global_delta = 0.1;
		pruneTime = 0.0;
		numPrune = 0;
	}

	void PruneAlphaPlane::updateCertsAndUses(int timeStamp){

		DEBUG_TRACE(cout << "PruneAlpha::updateCertsAndUses timeStamp " << timeStamp << endl;);

		prunePlanes();
		//cout << "Current Delta: " << global_delta << endl;
		updateCertsByDeltaDominance(global_delta, timeStamp); 
		updateUsesByCorners();
	}

	void PruneAlphaPlane::setDelta(double newDelta)
	{
		//cout << "Set Delta to " << newDelta << endl;
		DEBUG_TRACE(cout << "PruneAlphaPlane::setDelta newDelta " << newDelta << endl;);
		global_delta = newDelta;
	}

	bool PruneAlphaPlane::isMax(SharedPointer<AlphaPlane> alphaPlane)
	{
		SARSOPAlphaPlaneTuple* dataTuple = (SARSOPAlphaPlaneTuple *) alphaPlane->solverData;
		return dataTuple->maxMeta.size() != 0;
	}

	//Funtion: pruneUncertified
	//Functionality:
	//	to prune away all alphaPlanes which do not certify (or
	//	delta-certify)any belief points and not been useful
	//	at the corners
	//Parameters:
	//	NA
	//Returns:
	//	NA
	void PruneAlphaPlane::pruneNotCertedAndNotUsed(void)
	{
		list<SharedPointer<AlphaPlane> > next_planes;
		next_planes.clear();

		//iterate planes, only retain the ones which has certificates or has
		//	been useful at corners
		LISTFOREACH (SharedPointer<AlphaPlane>, try_pair, alphaPlanePool->planes) 
		{
			SharedPointer<AlphaPlane> try_alpha = *try_pair;
		
			SARSOPAlphaPlaneTuple *tempTuple = (SARSOPAlphaPlaneTuple *)(try_alpha->solverData);
			// original :
			//if(try_alpha->certifiedBeliefs.size()>0 || try_alpha->used>0 || try_alpha-> isMax > 0){
			// improved : do not remove max alpha
			if(tempTuple->certifiedBeliefs.size()>0 || try_alpha->used>0 || isMax(try_alpha) )
			{
				next_planes.push_back(try_alpha);
			}
			else
			{
/*				//SYLADDED 10092008 - disallow empty pools!
				// if try_alpha is the last alpha in the pool and there are no alphas selected
				// for next_planes yet, then push try_alpha onto next_planes and dont delete
				if ( ((alphaPlanePool->planes).back() == try_alpha) && (next_planes.size() == 0))
				{
#if DEBUGSYL_100908				
cout << "Last alpha in the pool (not deleting) for sval: " << alphaPlanePool->sval << endl;
#endif
					next_planes.push_back(try_alpha);
				} else
				{ */
					if(tempTuple->certed==0)
					{
						DEBUG_TRACE( cout << "Alpha Pruned:" << endl; );
						DEBUG_TRACE( try_alpha->alpha->write(cout) << endl; );

						//delete(try_alpha);
						// no need to delete it, as the smart pointer can handle the deletion
					}
					else if(tempTuple->certed<0)
					{
						printf("error: certed less than zero!!!!!!!!!!!\n");
					}
				//needs delete !!
				//}
			}
		}
		alphaPlanePool->planes = next_planes;
		lastPruneNumPlanes = alphaPlanePool->planes.size();
		if (lastPruneNumPlanes == 0)
		{
			cout << "ERROR: ALPHA PLANE POOL IS EMPTY! for sval (not known, edit this code...):" << endl ; //<< alphaPlanePool->sval << endl;
		}

	}

	// The Bounds class decides whether to prune or not depending on the sum of pruneTime for the set of PruneAlphaPlanes
	void PruneAlphaPlane::pruneDynamicDeltaVersion(int timeStamp, int& overPrune, int& underPrune)
	{
		DEBUG_TRACE(cout << "PruneAlpha::pruneDynamicDeltaVersion global_delta " << global_delta << endl;);
		DEBUG_TRACE(cout<< "alphaPlanePool->planes.size() :"<< alphaPlanePool->planes.size() << endl;);

		double curTime = GlobalResource::getInstance()->getRunTime();	
	
			overPrune = 0;
			underPrune = 0;		

			
			resetAlphaPlaneStateMachine();

			updateCertsAndUses(timeStamp);

			//Sanity();
			computePruneStats(&overPrune, &underPrune);

			pruneNotCertedAndNotUsed();

			double usedTime = GlobalResource::getInstance()->getRunTime() - curTime;
			pruneTime += usedTime;
			numPrune ++;


	}
	
	void PruneAlphaPlane::Sanity()
	{
		
		// check alpha max property
		int nAlpha = alphaPlanePool->planes.size();
		int nBelief =  alphaPlanePool->beliefCache->size();
		//int nBelief = GlobalResource::getInstance()->beliefCache->size();

		//double* pInner = new double[nAlpha * nBelief];
		
		LISTFOREACH(SharedPointer<AlphaPlane>,  try_pair,  alphaPlanePool->planes)
		{
			SharedPointer<AlphaPlane> try_alpha = *try_pair;
			SARSOPAlphaPlaneTuple *tempTuple = (SARSOPAlphaPlaneTuple *)(try_alpha->solverData);
			tempTuple->sanityMax = false;
		}
		
		for(int i = 0 ; i < alphaPlanePool->beliefCache->currentRowCount ; i ++)
		{
			double curMax = -DBL_MAX;
			SharedPointer<AlphaPlane> maxAlpha = NULL;
			const SharedPointer<belief_vector>  b = alphaPlanePool->beliefCache ->getRow(i)->BELIEF;
			
			LISTFOREACH(SharedPointer<AlphaPlane>,  try_pair,  alphaPlanePool->planes)
			{
				SharedPointer<AlphaPlane> try_alpha = *try_pair;
				double dotProductValue = inner_prod( *(try_alpha -> alpha),*b);
				if(dotProductValue > curMax )
				{
					curMax = dotProductValue;
					maxAlpha = try_alpha;
				}
			}

			SARSOPAlphaPlaneTuple *tempTuple = (SARSOPAlphaPlaneTuple *)(maxAlpha->solverData);
			tempTuple->sanityMax = true;
		}
		
		int maxCount = 0;
		LISTFOREACH(SharedPointer<AlphaPlane>,  try_pair,  alphaPlanePool->planes)
		{
			SharedPointer<AlphaPlane> try_alpha = *try_pair;
			SARSOPAlphaPlaneTuple *tempTuple = (SARSOPAlphaPlaneTuple *)(try_alpha->solverData);
			if(tempTuple->sanityMax == true)
			{
				maxCount ++;
			}
		}
		//cout << "Sanity :: Max Alpha : " << maxCount << endl;
	}

	void PruneAlphaPlane::updateCertsByDeltaDominance(double delta, int timeStamp)
	{
		DEBUG_TRACE(cout << "updateCertsByDeltaDominance" << endl;);

		LISTFOREACH(SharedPointer<AlphaPlane>,  try_pair,  alphaPlanePool->planes) 
		{
			SharedPointer<AlphaPlane> try_alpha = *try_pair;
			updateCerts(try_alpha, delta, timeStamp);
			

			if(isMax(try_alpha))
			{
				DEBUG_TRACE( cout << "isMax" << endl );
				updateMax(try_alpha, delta, timeStamp);
			}
		}
	}//end method: updateCertsByDeltaDominance

	void PruneAlphaPlane::resetAlphaPlaneStateMachine()
	{
		/*
		LISTFOREACH(SharedPointer<AlphaPlane>,  try_pair,  alphaPlanePool->planes) {
			SharedPointer<AlphaPlane> try_alpha = *try_pair;
			try_alpha -> misPruned = 0;
			try_alpha -> misKept = 0;
		}
		*/
	}

	
	void PruneAlphaPlane::computePruneStats(int *oP, int *uP)
	{
		
		int overPrune = 0;
		int underPrune = 0;
		
		int maxCount = 0;
		
		LISTFOREACH (SharedPointer<AlphaPlane>, try_pair, alphaPlanePool->planes)
		{
			
			SharedPointer<AlphaPlane> try_alpha = *try_pair;
			SARSOPAlphaPlaneTuple *tempTuple = (SARSOPAlphaPlaneTuple *)(try_alpha->solverData);
			int beliefSize = problem->getBeliefSize();
			
			if(isMax(try_alpha))
			{
				maxCount ++;
			}
			
			if( isMax(try_alpha) && tempTuple ->certed == 0)
			{
				overPrune ++;
			}
			if( !( isMax(try_alpha)) && tempTuple -> certed > 0)
			{
				underPrune ++;
			}
		}
		
		//cout << "Current Max Alpha : " << maxCount << endl;
		//cout << "Compute Prune Stats : OverPrune " << overPrune << " UnderPrune " << underPrune << endl;
		*oP = overPrune;
		*uP = underPrune;
	}

	
	void PruneAlphaPlane::updateUsesByCorners(){
		//reset uses, so that corner planes can be recorded
		resetUseds();

#if DEBUGSYL_061008_1
unsigned int planeCount = 0;
#endif
		
		if(alphaPlanePool->planes.size()>0)
		{
			int beliefSize = problem->getBeliefSize();
			//add in a structure holding all max values for each entry
			std::vector <double> cornerValues;
			//add in a structure holding all corner planes for each entry
			vector <SharedPointer<AlphaPlane> > cornerPlanes;
			//init corners
			//cornerValues.resize(beliefSize);
			//cornerPlanes.resize(beliefSize);
			for (int i = 0; i<beliefSize; i++)
			{
				cornerValues.push_back(-99e+20);
				cornerPlanes.push_back(NULL);
			}

#if DEBUGSYL_061008
for (int i = 0; i<beliefSize; i++)
{
	cout << "i : " << i << " : " << cornerValues.at(i);
	cout << " : " << cornerPlanes.at(i) << endl;
}
#endif
			//update max corner value and plane pointer iteratively
			LISTFOREACH(SharedPointer<AlphaPlane>,  plane,  alphaPlanePool->planes) {
				updateCorners(*plane, &cornerValues, &cornerPlanes);//to be added later
			}

#if DEBUGSYL_061008
for (int i = 0; i<beliefSize; i++)
{
	cout << "i : " << i << " : " << cornerValues.at(i);
	cout << " : " << cornerPlanes.at(i) << endl;
}
#endif

			//update uses for corners
			for (int i = 0; i<(int)cornerPlanes.size(); i++)
			{
				SharedPointer<AlphaPlane> plane = cornerPlanes.at(i);

				if(plane != NULL){
					plane->used++;

#if DEBUGSYL_061008_1
 planeCount++;
#endif

				}
			}
		}
#if DEBUGSYL_061008_1
else
cout << "alphaPlanePool->planes.size() !> 0 !!!" <<endl;
#endif


#if DEBUGSYL_061008_1
 cout << " planeCount : " << planeCount << endl;
#endif

	}//end method: updateUsesByCorners

	void PruneAlphaPlane::updateCorners(SharedPointer<AlphaPlane> plane, std::vector<double, std::allocator<double> >* values, std::vector<SharedPointer<AlphaPlane>,  std::allocator<SharedPointer<AlphaPlane> > >* indices)
	{
		SharedPointer<alpha_vector> alpha = plane->alpha;
		for (int i = 0; i<(int)alpha->data.size(); i++)
		{
			// TODO: determine alpha vector type at compile time
			// cvector version
			// 			int index = alpha.data.at(i).index;
			// 			double value = alpha.data.at(i).value;
			// 			if(value > values->at(index)){
			// 				values->at(index)=value;
			// 				indices->at(index)=plane;
			// 			}

			// dvector version, added by Yanzhu 14 Aug 2007
			int index = i;
			double value = alpha->data[i];
			if(value > values->at(index))
			{
				values->at(index)=value;
				indices->at(index)=plane;
			}
		}
	}

	void PruneAlphaPlane::updateMax(SharedPointer<AlphaPlane> plane, double delta, int timeStamp)
	{
		vector<AlphaPlaneMaxMeta*> nextMaxMeta;

		SARSOPAlphaPlaneTuple *planeTuple = (SARSOPAlphaPlaneTuple *)(plane->solverData);
		
		FOREACH(AlphaPlaneMaxMeta* , entry, planeTuple->maxMeta)
		{
			int i = (*entry)->cacheIndex;
			bool removeThis = false;
			int maxTimeStamp;
			//if(alphaPlanePool->beliefCache ->getRow(i)->isFringe)
			if(alphaPlanePool->beliefCache ->getRow(i)->REACHABLE->isFringe())
			{
				const SharedPointer<belief_vector>  b = alphaPlanePool->beliefCache ->getRow(i)->BELIEF;
				//const SharedPointer<belief_vector>  b = GlobalResource::getInstance()->beliefCache ->getRow(i)->BELIEF;
				
				int lastTimeStamp = (*entry)->timestamp;
				maxTimeStamp = lastTimeStamp;
				LISTFOREACH(SharedPointer<AlphaPlane>,  try_pair,  alphaPlanePool->planes) 
				{
					SharedPointer<AlphaPlane> try_alpha = * try_pair;
					//only not examined alphas are examined
					if(try_alpha->timeStamp > lastTimeStamp)
					{
						double dotProductValue = inner_prod( *(try_alpha -> alpha),*b);
						if(dotProductValue > (*entry)->lastLB )
						{
							removeThis = true;
							break;
						}
	
						if( try_alpha->timeStamp > maxTimeStamp)
						{
							maxTimeStamp = try_alpha->timeStamp;
						}
					}
				}
			}
			else
			{
				removeThis = true;
			}

			if(!removeThis )
			{
				(*entry)->timestamp = maxTimeStamp;
			}
			if(!removeThis)
			{
				nextMaxMeta.push_back((*entry));
			}
			else
			{
				delete (*entry);
			}
		}

		planeTuple->maxMeta = nextMaxMeta;

	}

	
	/* updateMax "Move version" 
	void PruneAlphaPlane::updateMax(SharedPointer<AlphaPlane> plane, double delta, int timeStamp)
	{
		vector<AlphaPlaneMaxMeta*> nextMaxMeta;
		FOREACH(AlphaPlaneMaxMeta* , entry, plane->maxMeta)
		{
			int i = (*entry)->cacheIndex;
			bool removeThis = false;
			const SharedPointer<belief_vector>  b = GlobalResource::getInstance()->beliefCache ->getRow(i)->BELIEF;
			int lastTimeStamp = (*entry)->timestamp;
			int maxTimeStamp = lastTimeStamp;
			LISTFOREACH(SharedPointer<AlphaPlane>,  try_pair,  alphaPlanePool->planes) 
			{
				SharedPointer<AlphaPlane> try_alpha = * try_pair;
				if(plane == try_alpha)
				{
					continue;
				}
				//only not examined alphas are examined
				if(try_alpha->timeStamp > lastTimeStamp)
				{
					double dotProductValue = inner_prod( try_alpha -> alpha,*b);
					if(dotProductValue > (*entry)->lastLB )
					{
						removeThis = true;
						if(try_alpha->hasMaxMetaAt((*entry)->cacheIndex))
						{
							delete (*entry);
						}
						else
						{
							try_alpha->maxMeta.push_back((*entry));
							(*entry)->timestamp = lastTimeStamp;
						}
						
						break;
					}

					
					if( try_alpha->timeStamp > maxTimeStamp)
					{
						maxTimeStamp = try_alpha->timeStamp;
					}
				}
			}

			
			if(!removeThis )
			{
				(*entry)->timestamp = maxTimeStamp;
			}
			
			
			if(!removeThis)
			{
				nextMaxMeta.push_back((*entry));
			}
			
			
			//else
			//{
			//	delete (*entry);
			//}
			
			
		}

		plane->maxMeta = nextMaxMeta;

	}
	*/
	

	void PruneAlphaPlane::updateCerts(SharedPointer<AlphaPlane> plane, double delta, int timeStamp)
	{
		DEBUG_TRACE(cout << "updateCerts" << endl;);


		SARSOPAlphaPlaneTuple *planeTuple = (SARSOPAlphaPlaneTuple *)(plane->solverData);
		//for each previously certifing BeliefTreeNode, check whether it still dominates
		int oldCertState = planeTuple -> certed;

		for(int i=0; i<(int)planeTuple->certifiedBeliefs.size(); i++)
		{

			DEBUG_TRACE(cout << "i " << i << endl;);

			BeliefTreeNode* n = planeTuple->certifiedBeliefs.at(i);
			SharedPointer<belief_vector>  b = n->s->bvec;

			int lastTimeStamp, maxTimeStamp;
			double value, maxValue;
			bool dominated;
			if(n->count > 0)
			{

				DEBUG_TRACE(cout << "n->count " << n->count << endl;);

				//check whether it is still worth retaining certificate
				lastTimeStamp = planeTuple->certifiedBeliefTimeStamps.at(i);
				maxTimeStamp = lastTimeStamp;
				value = inner_prod( *(plane->alpha), *b);
				maxValue = value;
				dominated = false;
				SharedPointer<AlphaPlane> maxPlane = NULL;//may cause trouble later
				LISTFOREACH(SharedPointer<AlphaPlane>,  try_pair,  alphaPlanePool->planes) 
				{
					SharedPointer<AlphaPlane> try_alpha = * try_pair;


#if USE_MASKED_ALPHA
					if (!mask_subset( *b, try_alpha->mask )) continue;
#endif
					//only not examined alphas are examined

					DEBUG_TRACE(cout << "try_alpha->timeStamp " << try_alpha->timeStamp << endl;);
					DEBUG_TRACE(cout << "lastTimeStamp " << lastTimeStamp << endl;);
					
					if(try_alpha->timeStamp > lastTimeStamp)
					{
						alpha_vector alpha_distance;
						// TODO:: see if can eliminate this vector copy
						copy(alpha_distance,  *(plane->alpha));
						double newValue = inner_prod( *(try_alpha->alpha),*b);

						DEBUG_TRACE(cout << "maxValue " << maxValue << endl;);
						DEBUG_TRACE(cout << "newValue " << newValue << endl;);

						if(maxValue < newValue-0.00000000000001)
						{
							DEBUG_TRACE(cout << "smaller " << endl;);
							//store delta statistics
							if (maxPlane != NULL)
							{
								DEBUG_TRACE(cout << "maxPlane != NULL " << endl;);
								copy(alpha_distance, *(maxPlane->alpha));
							}

							alpha_distance -=( *(try_alpha->alpha));
							double squareDelta = (newValue - maxValue)*	(newValue-maxValue)/(alpha_distance.norm_2());
							double deltaValue = sqrt(squareDelta);
							//check dominance
							if (deltaValue > delta){
								maxPlane = try_alpha;
								dominated = true;
								maxValue = newValue;
							}
						}
						//update maxTimeStamp when needed
						if (try_alpha->timeStamp > maxTimeStamp)
						{
							maxTimeStamp = try_alpha->timeStamp;
						}
					}
				}

				//do updates of attributes for n, plane and maxPlaneif dominated
				if(dominated)
				{
					maxPlane->addDominatedBelief(maxTimeStamp, n);
					planeTuple->certifiedBeliefs.at(i)= NULL;
				}
			}
			else
			{
				planeTuple->certifiedBeliefs.at(i)= NULL;
			}
		}
		//erase beliefs which no longer dominates
		int size = planeTuple->certifiedBeliefs.size();
		vector<BeliefTreeNode*> remainingBeliefs;
		vector<int> remainingTimeStamps;
		for(int i = 0; i<size; i++){
			BeliefTreeNode* n = planeTuple->certifiedBeliefs.at(i);
			int t = planeTuple->certifiedBeliefTimeStamps.at(i);
			if(n != NULL){
				remainingBeliefs.push_back(n);
				remainingTimeStamps.push_back(t);
			}
		}
		if( planeTuple->certifiedBeliefs.size()>0 && remainingBeliefs.size()==0)
		{
		}
		planeTuple->certifiedBeliefs = remainingBeliefs;
		planeTuple->certifiedBeliefTimeStamps = remainingTimeStamps;

		// Hanna's algorithm
		/*
		int newCertState = plane->certed;
		//cout << "oldCertState " << oldCertState << " newCertState " << newCertState << endl;
		if(newCertState == 0)
		{
			//cout << "Loose all certs" << endl;
			if(plane -> misPruned == 0)
			{
				plane -> misPruned = 1;
			}
		}
		*/
		/*
		if(newCertState > 0 && plane -> isMax == 0)
		{
		if(plane -> misKept == 0)
		{
		plane -> misKept = 1;
		}
		}
		*/
	}


	int PruneAlphaPlane::countCerts(void)
	{

		int total = 0;
		LISTFOREACH(SharedPointer<AlphaPlane>,  plane,  alphaPlanePool->planes) 
		{
			SharedPointer<AlphaPlane> alpha = *plane;
			SARSOPAlphaPlaneTuple *planeTuple = (SARSOPAlphaPlaneTuple *)(alpha->solverData);
			total += planeTuple->certifiedBeliefs.size();
		}
		return total;
	}

	//functionality:
	//	to count and return the number of planes that are certed
	int PruneAlphaPlane::countCertedPlanes(void){
		int total = 0;
		LISTFOREACH(SharedPointer<AlphaPlane>,  plane,  alphaPlanePool->planes) {
			SharedPointer<AlphaPlane> alpha = *plane;
			SARSOPAlphaPlaneTuple *planeTuple = (SARSOPAlphaPlaneTuple *)(alpha->solverData);
			if (planeTuple->certifiedBeliefs.size()>0)
			{
				total++;
			}
		}
		return total;
	}

	int PruneAlphaPlane::countUses(void){
		int total = 0;
		LISTFOREACH(SharedPointer<AlphaPlane>,  plane,  alphaPlanePool->planes) 
		{
			SharedPointer<AlphaPlane> alpha = *plane;
			total += alpha->used;
		}
		return total;
	}

	int PruneAlphaPlane::countUsedPlanes(void){
		int total = 0;
		LISTFOREACH(SharedPointer<AlphaPlane>,  plane,  alphaPlanePool->planes) 
		{
			SharedPointer<AlphaPlane> alpha = *plane;
			SARSOPAlphaPlaneTuple *planeTuple = (SARSOPAlphaPlaneTuple *)(alpha->solverData);
			if(alpha->used >0 && planeTuple->certifiedBeliefs.size()==0)
			{
				total++;
			}
		}
		return total;
	}

	void PruneAlphaPlane::resetUseds(void)
	{
		LISTFOREACH(SharedPointer<AlphaPlane>,  try_pair,  alphaPlanePool->planes) 
		{
			SharedPointer<AlphaPlane> try_alpha = *try_pair;
			try_alpha->used = 0;
		}
	}

	//most used pruning methods
	void PruneAlphaPlane::prunePlanes(void)
	{
		list<SharedPointer<AlphaPlane> > next_planes;
		list<list<SharedPointer<AlphaPlane> >::iterator> erase_ptrs;
		bool try_dominates_in, in_dominates_try;

		LISTFOREACH_NOCONST(SharedPointer<AlphaPlane>,  try_pair,  alphaPlanePool->planes) 
		{
			SharedPointer<AlphaPlane> try_alpha = *try_pair;
			erase_ptrs.clear();
			LISTFOREACH_NOCONST(SharedPointer<AlphaPlane>,  in_pair,  next_planes) 
			{
				SharedPointer<AlphaPlane> in_alpha = *in_pair;

#if USE_MASKED_ALPHA
				try_dominates_in = mask_dominates(try_alpha->alpha, in_alpha->alpha, ZMDP_BOUNDS_PRUNE_EPS,
					try_alpha->mask,  in_alpha->mask);
#else
				try_dominates_in = dominates(*try_alpha->alpha, *in_alpha->alpha, ZMDP_BOUNDS_PRUNE_EPS);
#endif
				if (try_dominates_in) 
				{
					// delay erasure since we're still iterating through the set
					erase_ptrs.push_back(in_pair);
				}
				else
				{
#if USE_MASKED_ALPHA
					in_dominates_try = mask_dominates(in_alpha->alpha, try_alpha->alpha, ZMDP_BOUNDS_PRUNE_EPS,
						in_alpha->mask,  try_alpha->mask);
#else
					in_dominates_try = dominates(*in_alpha->alpha, *try_alpha->alpha, ZMDP_BOUNDS_PRUNE_EPS);
#endif
					if (in_dominates_try) goto next_try_pair;
				}
			}

			// resolve delayed erasure
			LISTFOREACH_NOCONST(list<SharedPointer<AlphaPlane> >::iterator,  erase_ptr,  erase_ptrs) 
			{
				list<SharedPointer<AlphaPlane> >::iterator  x, xp1;
				SharedPointer<AlphaPlane> testDeleteAlphaPlane = (**erase_ptr);
				SARSOPAlphaPlaneTuple *planeTuple = (SARSOPAlphaPlaneTuple *)(testDeleteAlphaPlane->solverData);
				if(planeTuple->certed==0)
				{
					//delete testDeleteAlphaPlane;
					// no need to delete as the smart pointer should be able to handle it
				}
				else if(planeTuple->certed<0)
				{
					printf("error2: certed num less than 0!!!!!!!!!!\n");
				}
				x = xp1 = (*erase_ptr);
				xp1++;
				next_planes.erase(x,xp1);
			}
			next_planes.push_back(try_alpha );
next_try_pair: ;
		}
#if USE_DEBUG_PRINT
		cout << "... pruned # planes from " << planes.size()
			<< " down to " << next_planes.size() << endl;
#endif
		alphaPlanePool->planes = next_planes;
		lastPruneNumPlanes = alphaPlanePool->planes.size();
	}

	// prune planes if the number has grown significantly
	// since the last check
	void PruneAlphaPlane::prune(void)
	{
		unsigned int nextPruneNumPlanes = max(lastPruneNumPlanes + 10,
			(int) (lastPruneNumPlanes * 1.1));
		if (alphaPlanePool->planes.size() > nextPruneNumPlanes) 
		{
#if DEBUGSYL_100908
cout <<"Prune() for sval : " << alphaPlanePool->sval << " alphaPlanePool->planes.size() :"<< alphaPlanePool->planes.size() << endl;
#endif
			prunePlanes();
#if DEBUGSYL_100908
cout <<"After calling prunePlanes(), alphaPlanePool->planes.size() :"<< alphaPlanePool->planes.size() << endl;
#endif
			if (alphaPlanePool->planes.size() == 0)
				cout << "ERROR: ALPHA PLANE POOL IS EMPTY! for sval (edit code):" << endl; //alphaPlanePool->sval << endl;

		}
	}
};

