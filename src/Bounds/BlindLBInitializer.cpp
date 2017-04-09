/** 
* Part of the this code is derived from ZMDP: http://www.cs.cmu.edu/~trey/zmdp/
* ZMDP is released under Apache License 2.0
* The rest of the code is released under GPL v2 
*/


#include <stdlib.h>
//#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include <iostream>
#include <fstream>

#include "MOMDP.h"
#include "SARSOP.h"
#include "BlindLBInitializer.h"

using namespace std;

using namespace momdp;

#define PRUNE_EPS (1e-10)

//#define DEBUGSYL_180908 1
//#define DEBUGSYL_310708 1
//#define DEBUG_LBVECTOR_220908 1
//#define DEBUG_ITERCOMP_220908 1

namespace momdp {

	BlindLBInitializer::BlindLBInitializer(SharedPointer<MOMDP> _pomdp, AlphaPlanePoolSet* _bound) {
		pomdp = (SharedPointer<MOMDP>) _pomdp;
		bound = _bound;
	}

	void BlindLBInitializer::initialize(double targetPrecision)
	{

		if(pomdp->XStates->size() != 1 && pomdp->hasPOMDPMatrices())
		{
			// only does this if convert fast is called to produce pomdp version of the matrices
			initBlind_unfac(targetPrecision);
		}
		else
		{
			initBlind(targetPrecision);
		}
	}


	void BlindLBInitializer::initBlindWorstCase_unfac(alpha_vector& weakAlpha)
	{
		DEBUG_TRACE(cout << "BlindLBInitializer::initBlindWorstCase_unfac " << endl; );
		// set alpha to be a lower bound on the value of the best blind policy

		double worstStateVal;
		int safestAction = -1;
		double worstCaseReward = -99e+20;
		// calculate worstCaseReward = max_a min_s R(s,a)
		// safestAction = argmax_a min_s R(s,a)
		int numStatesObs = pomdp->XStates->size();
		int numStatesUnobs = pomdp->YStates->size();
		FOR (a, pomdp->getNumActions()) 
		{
			worstStateVal = 99e+20;

			//cmatrix* pR = pomdp->pomdpR;
			
			FOR (s, numStatesObs * numStatesUnobs) 
			{
				//     worstStateVal = std::min(worstStateVal, (*pR)(s,a));
				worstStateVal = std::min(worstStateVal, (*(pomdp->pomdpR))(s,a));
			}
			if (worstStateVal > worstCaseReward) {
				safestAction = a;
				worstCaseReward = worstStateVal;
			}
		}
		//DenseVector worstCaseDVector(pomdp->getNumStateDimensions());


		DenseVector worstCaseDVector(numStatesObs*numStatesUnobs);
		assert(pomdp->getDiscount() < 1);
		double worstCaseLongTerm = worstCaseReward / (1 - pomdp->getDiscount());
		FOR (i, numStatesObs*numStatesUnobs) 
		{
			worstCaseDVector(i) = worstCaseLongTerm;
		}

		// post-process: make sure the value for all terminal states
		// is exactly 0, since that is how the lbVal field of terminal
		// nodes is initialized.
		FOR (i, numStatesObs*numStatesUnobs) {

			// convert i to x and y values
			unsigned int x = (unsigned int) i/(numStatesUnobs);
			unsigned int y = i % (numStatesUnobs);

			if (pomdp->isPOMDPTerminalState[x][y]) {

				worstCaseDVector(i) = 0.0;
			}
		}

		copy(weakAlpha, worstCaseDVector);
		
		DEBUG_TRACE(cout << "weakAlpha" << endl; );
		DEBUG_TRACE( weakAlpha.write(cout) << endl; );
	}

	void BlindLBInitializer::initBlind_unfac(double targetPrecision)
	{
		DEBUG_TRACE(cout << "BlindLBInitializer::initBlind_unfac " << targetPrecision << endl; );
		
		int numStatesObs = pomdp->XStates->size();
		int numStatesUnobs = pomdp->YStates->size();
		alpha_vector al(numStatesObs*numStatesUnobs);
		alpha_vector nextAl, tmp, diff;
		alpha_vector weakAl;
		belief_vector dummy; // ignored
		double maxResidual;

		std::vector<alpha_vector> als(numStatesObs);

		initBlindWorstCase_unfac(weakAl);
		//bound->planes.clear();


		// TODO:: bound->elapsed = bound->heurTimer.elapsed();
		// TODO:: printf("%.2fs initBlindWorstCase(sval, tmpAl) done for all svals\n", bound->elapsed);


		// produce one alpha vector for each fixed policy "always take action a"
		FOR (a, pomdp->actions->size()) 
		{
		    DEBUG_TRACE(cout << "a " << a << endl; );
			al = weakAl;

			do 
			{
				// calculate nextAl
				mult(nextAl, *(*(pomdp->pomdpT))[a], al);
//				DEBUG_TRACE(cout << "al " << endl; );
//				DEBUG_TRACE(al.write(cout) << endl; );
				
//				DEBUG_TRACE(cout << "nextAl " << endl; );
//				DEBUG_TRACE(nextAl.write(cout) << endl; );
				
				//     mult(nextAl, pomdp->pomdpT[a], al);
				nextAl *= pomdp->discount;
				
//				DEBUG_TRACE(cout << "nextAl " << endl; );
//				DEBUG_TRACE(nextAl.write(cout) << endl; );
				
				copy_from_column(tmp, *(pomdp->pomdpR), a);
				//      copy_from_column(tmp, pomdp->pomdpR, a);
//				DEBUG_TRACE(cout << "tmp " << endl; );
//				DEBUG_TRACE(tmp.write(cout) << endl; );
				
				nextAl += tmp;
				
//				DEBUG_TRACE(cout << "nextAl " << endl; );
//				DEBUG_TRACE(nextAl.write(cout) << endl; );

				// calculate residual
				diff = nextAl;
				diff -= al;
				
//				DEBUG_TRACE(cout << "diff " << endl; );
//				DEBUG_TRACE(diff.write(cout) << endl; );
				
				maxResidual = diff.norm_inf();

				al = nextAl;
				
				DEBUG_TRACE(cout << "maxResidual " << maxResidual << endl; );
				DEBUG_TRACE(cout << "targetPrecision " << targetPrecision << endl; );
			} while (maxResidual > targetPrecision);


#if USE_DEBUG_PRINT
			cout << "initLowerBoundBlind: a=" << a << " al=" << sparseRep(al) << endl;
#endif

			/* #if USE_MASKED_ALPHA
			bound->addAlphaPlane(dummy, al, a, full_mask);
			#else
			SharedPointer<AlphaPlane> plane = new AlphaPlane(al, a);
			plane->setTimeStamp(0); 
			plane->certed = 0;//init certed count to 0
			bound->addAlphaPlane(plane); 
			// bound->addAlphaPlane(dummy, al, a); //!!to be added in later
			#endif */

			// break al up into als[sval] for each sval 
			FOR (s, numStatesObs * numStatesUnobs) 
			{
				// convert i to x and y values
				unsigned int x = (unsigned int) s/(numStatesUnobs);
				unsigned int y = s % (numStatesUnobs);

				if (y==0) als[x].resize(numStatesUnobs);

				als[x](y) = al(s);
			}

			FOR (sval, numStatesObs ) 
			{
				if (a == 0) 
				{
					bound->set[sval]->planes.clear();
				}

				DEBUG_TRACE(cout << "als[sval] " << sval << endl; );
				DEBUG_TRACE(als[sval].write(cout) << endl; );
				
				SharedPointer<AlphaPlane> plane (new AlphaPlane());
				copy(*plane->alpha, als[sval]);
				plane->action = a;
				plane->sval = sval;
				plane->setTimeStamp(0); 

				SARSOPAlphaPlaneTuple *tempTuple = (SARSOPAlphaPlaneTuple *)plane->solverData;
				tempTuple->certed = 0; //init certed count to 0
				
				bound->set[sval]->addAlphaPlane(plane);
				// bound->addAlphaPlane(dummy, al, a); //!!to be added in later

				/*cout << "a " << a << " sval " << sval << endl;
				als[sval].write(std::cout);
				cout << endl;*/
			}	

		}

		//bound->elapsed = bound->heurTimer.elapsed();
		//printf("%.2fs initBlind() done\n", bound->elapsed);
	}

	// SYL260809 new
	// this gets the worst case initial vector separately for each sval and writes to the set of weakAlpha vectors
	void BlindLBInitializer::initBlindWorstCase(std::vector<alpha_vector>& weakAlphas)
	{
		DEBUG_TRACE(cout << "BlindLBInitializer::initBlindWorstCase " << endl; );
		
		// set alpha to be a lower bound on the value of the best blind policy

		double worstStateVal;
		int safestAction = -1;
		double worstCaseReward;
		// calculate worstCaseReward = max_a min_s R(s,a)
		// safestAction = argmax_a min_s R(s,a)
		int numStates = pomdp->YStates->size();
		int numStatesObs = pomdp->XStates->size();

		double worstCaseLongTerm; 
		assert(pomdp->getDiscount() < 1);

		FOR (sval, numStatesObs) 
		{	
			worstCaseReward = -99e+20;
			SharedPointer<SparseMatrix>  rewardMatrix = pomdp->rewards->getMatrix(sval);
	
			FOR (a, pomdp->getNumActions()) 
			{
				worstStateVal = 99e+20;

				FOR (s, numStates) 
				{
					worstStateVal = std::min(worstStateVal, rewardMatrix->operator ()(s,a));
					//worstStateVal = std::min(worstStateVal, (*(pomdp->rewards->matrix[sval]))(s,a));
				}
				if (worstStateVal > worstCaseReward) 
				{
					safestAction = a;
					worstCaseReward = worstStateVal;
				}
			}

// OPTION A, works better than OPTION B for some problems - gets better lowerbound vectors.
			// if safestAction and sval combination transits to a terminal state
			// then, do not multiply with long term discount effect
			bool penultimateTerminal = true;
			bool isTerminal;
			SparseVector T_xa;
	
			FOR (y, numStates) 
			{
				copy_from_column( T_xa, *(pomdp->XTrans->getMatrixTr(safestAction, sval)), y);
	
				FOR (nonZeroEntry,T_xa.data.size()) {
	
					isTerminal = true;
					// need to make sure that ALL (regardless of y' value) are terminal states
					FOR (yn, numStates) 
					{
						if (!(pomdp->isPOMDPTerminalState[T_xa.data[nonZeroEntry].index][yn])) {
							isTerminal = false;
							break;
	
						}
					}
	
					if (!isTerminal) {
						penultimateTerminal = false;
						break;
					}
				}
			}
	
			if (penultimateTerminal) {
				worstCaseLongTerm = worstCaseReward;
			} else {
				assert(pomdp->getDiscount() < 1);
				worstCaseLongTerm = worstCaseReward / (1 - pomdp->getDiscount());
			}

// OR, OPTION B
//			worstCaseLongTerm = worstCaseReward / (1 - pomdp->getDiscount());

			weakAlphas[sval].resize(numStates);
			FOR (i, numStates) 
			{
				// post-process: make sure the value for all terminal states
				// is exactly 0, since that is how the lbVal field of terminal
				// nodes is initialized.
				if (pomdp->isPOMDPTerminalState[sval][i]) 
				{
					weakAlphas[sval](i) = 0.0;
				}
				else
				{
					weakAlphas[sval](i) = worstCaseLongTerm;
				}

			}

		}

	}

	// this gets the worst case initial vector acros all sval
	int BlindLBInitializer::initBlindWorstCaseIntegrated(std::vector<alpha_vector>& weakAlphas)
	{
		DEBUG_TRACE(cout << "BlindLBInitializer::initBlindWorstCaseIntegrated" << endl; );
		
		// set alpha to be a lower bound on the value of the best blind policy
		double worstStateVal;
		int safestAction = -1;
		double worstCaseReward = -99e+20;
		int numStates = pomdp->YStates->size();
		int numStatesObs = pomdp->XStates->size();
		// calculate worstCaseReward = max_a min_s R(s,a)
		// safestAction = argmax_a min_s R(s,a)
		FOR (a, pomdp->getNumActions()) 
		{
			worstStateVal = 99e+20;

			FOR (sval, numStatesObs) 
			{	
				SharedPointer<SparseMatrix>  rewardMatrix = pomdp->rewards->getMatrix(sval);
				FOR (s, numStates) 
				{
					worstStateVal = std::min(worstStateVal, rewardMatrix->operator ()(s,a));
				}
			}
			if (worstStateVal > worstCaseReward) 
			{
				safestAction = a;
				worstCaseReward = worstStateVal;
			}
		}


		double worstCaseLongTerm; 

		assert(pomdp->getDiscount() < 1);
		worstCaseLongTerm = worstCaseReward / (1 - pomdp->getDiscount());

		FOR (sval, numStatesObs)
		{
			weakAlphas[sval].resize(numStates);
			FOR (i, numStates) 
			{
				// post-process: make sure the value for all terminal states
				// is exactly 0, since that is how the lbVal field of terminal
				// nodes is initialized.
				if (pomdp->isPOMDPTerminalState[sval][i]) 
				{
					weakAlphas[sval](i) = 0.0;
					//weakAlphas[sval].push_back = 0.0;

				}
				else
				{
					weakAlphas[sval](i) = worstCaseLongTerm;
				}
				//weakAlphas[sval].push_back = worstCaseLongTerm;

			}
			
			DEBUG_TRACE(cout << "weakAlphas[sval] " << sval << endl; );
			DEBUG_TRACE( weakAlphas[sval].write(cout)  << endl; );
		}

		DEBUG_TRACE(cout << "safestAction " << safestAction << endl; );

		return safestAction;
	}


	void BlindLBInitializer::initBlind(double targetPrecision)
	{
		DEBUG_TRACE(cout << "BlindLBInitializer::initBlind " << targetPrecision << endl; );
		
		int numStatesObs = pomdp->XStates->size();
		int numStatesUnobs = pomdp->YStates->size();


		std::vector<alpha_vector> als(numStatesObs), nextAls(numStatesObs), weakAls(numStatesObs);
		alpha_vector tmp, diff, tmpAl, tmpNextAl, tmpNextAl1;
		//belief_vector dummy; // ignored
		double maxResidual, maxResidualthis;

		initBlindWorstCaseIntegrated(weakAls); // SYL280809 prevly  //initBlindWorstCase(weakAls);

		//  TODO:: bound->elapsed = bound->heurTimer.elapsed();
		//  TODO:: printf("%.2fs initBlindWorstCase(sval, tmpAl) done for all svals\n", bound->elapsed);

		// produce one alpha vector for each fixed policy "always take action a"
		FOR (a, pomdp->actions->size()) 
		{
			als = weakAls;


			// SYLMOD use weak bounds
			do 
			{
				maxResidual = 0; 
				FOR (sval, numStatesObs ) {	// for each sval, get nextAl[sval]

					tmpAl.resize(numStatesUnobs);

                                        const vector<int>& possibleNextSvals = pomdp->XTrans->getMatrix(a, sval)->nonEmptyColumns();

					// calculate nextAl[sval]
                                        // if TX[a][sval] is all zero, dont do any multiplication, and dont need
                                        // to add anything to tmpAl
					FOREACH (int, nextSvalIt, possibleNextSvals) 
                                        {
                                            int nextSval = *nextSvalIt;
                                            mult(tmpNextAl, *pomdp->YTrans->getMatrix(a, sval, nextSval), als[nextSval]);
                                            emult_column( tmpNextAl1, *pomdp->XTrans->getMatrix(a, sval), nextSval, tmpNextAl );
                                            tmpAl += tmpNextAl1;
                                        }

					tmpAl *= pomdp->discount;

					copy_from_column(tmp, *(pomdp->rewards->getMatrix(sval)), a);
					tmpAl += tmp;

					// calculate residual
					diff = tmpAl;
					diff -= als[sval];
					maxResidualthis = diff.norm_inf();
					if (maxResidualthis > maxResidual)	
						maxResidual = maxResidualthis;

					nextAls[sval] = tmpAl;
				}

				als = nextAls;	

				DEBUG_TRACE(cout << "maxResidual " << maxResidual << endl; );
				//cout << "maxResidual " << maxResidual << endl;
				DEBUG_TRACE(cout << "targetPrecision " << targetPrecision << endl; );
			} while (maxResidual > targetPrecision);


			FOR (sval, numStatesObs ) 
			{
				if (a == 0) 
				{
					bound->set[sval]->planes.clear();
				}

				DEBUG_TRACE(cout << "als[sval] " << sval << endl; );
				DEBUG_TRACE(als[sval].write(cout) << endl; );

				SharedPointer<AlphaPlane> plane (new AlphaPlane());
				copy(*plane->alpha, als[sval]);
				plane->action = a;
				plane->sval = sval;


				plane->setTimeStamp(0); 

				
				SARSOPAlphaPlaneTuple *tempTuple = (SARSOPAlphaPlaneTuple *)plane->solverData;
				tempTuple->certed = 0; //init certed count to 0

				bound->set[sval]->addAlphaPlane(plane);
				// bound->addAlphaPlane(dummy, al, a); //!!to be added in later

			}	
		}


	}

}; // namespace momdp

