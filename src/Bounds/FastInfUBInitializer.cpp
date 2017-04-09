/** 
* Part of the this code is derived from ZMDP: http://www.cs.cmu.edu/~trey/zmdp/
* ZMDP is released under Apache License 2.0
* The rest of the code is released under GPL v2 
*/


#include <stdlib.h>
#ifdef _MSC_VER
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <assert.h>

#include <iostream>
#include <fstream>

#include "MOMDP.h"
#include "FastInfUBInitializer.h"
#include "FullObsUBInitializer.h"
#include "BeliefValuePairPoolSet.h"

using namespace std;
//using namespace MatrixUtils;

//#define DEBUGSYL_290908 1
//#define DEBUGSYL_160908a 1

namespace momdp {

	FastInfUBInitializer::FastInfUBInitializer( SharedPointer<MOMDP> problem, BeliefValuePairPoolSet* _bound)
	{
		pomdp = ( SharedPointer<MOMDP>) problem;
		bound = _bound;

		// pomdp->XStates->size()
		//alphasByState.resize(pomdp->XStates->size());   // SYL260809 commented out
		//upperBoundBVpair = _upperBoundBVpair;

	}

	FastInfUBInitializer::FastInfUBInitializer(SharedPointer<MOMDP> problem)
	{
		pomdp = ( SharedPointer<MOMDP>) problem;

		// pomdp->XStates->size()
		//alphasByState.resize(pomdp->XStates->size());   // SYL260809 commented out
		//upperBoundBVpair = _upperBoundBVpair;

	}

	void FastInfUBInitializer::initialize(double targetPrecision)
	{
		if(pomdp->XStates->size() != 1 && pomdp->hasPOMDPMatrices())
		{
			// only does this if convert fast is called to produce pomdp version of the matrices
			initFIB_unfac(targetPrecision, false); // need pomdp matrix
		}
		else
		{
			initFIB(targetPrecision, false);
		}

		//if (pomdp->XStates->size() == 1)
		//	initFIB(targetPrecision);
		//else
		//	initFIB_unfac(targetPrecision);
	}

	void FastInfUBInitializer::getFIBsolution(double targetPrecision)
	{
		if(pomdp->XStates->size() != 1 && pomdp->hasPOMDPMatrices())
		{
			// only does this if convert fast is called to produce pomdp version of the matrices
			initFIB_unfac(targetPrecision, true); // need pomdp matrix
		}
		else
		{
			initFIB(targetPrecision, true);
		}

		//if (pomdp->XStates->size() == 1)
		//	initFIB(targetPrecision);
		//else
		//	initFIB_unfac(targetPrecision);
	}


	void FastInfUBInitializer::initMDP_unfac(double targetPrecision)
	{
		// set alpha to be the mdp upper bound
		FullObsUBInitializer m;
		m.valueIteration_unfac(pomdp, targetPrecision);
		DenseVector calpha;
		copy(calpha, m.alpha);

		alphas.clear();
		alphas.push_back(calpha);

		// **** ADDED PRINTOUTS TO SCREEN HERE ******** 6 APRIL 2009 *******
		/*  cout << pomdp->YStates->size() << endl;
		FOR (i, pomdp->YStates->size()) {
		cout << calpha(i) << " ";
		}
		cout << endl;
		*/

	}

	void FastInfUBInitializer::initFIB_unfac(double targetPrecision, bool getFIBvectors)
	{
		// calculates the fast informed bound (Hauskrecht, JAIR 2000)
		std::vector< DenseVector > al(pomdp->actions->size());
		std::vector< DenseVector > nextAl(pomdp->actions->size());
		DenseVector tmp, beta_aoi, beta_ao, diff;
		double maxResidual;
		alpha_vector backup;

		initMDP_unfac(MDP_RESIDUAL);

		// TODO:: bound->elapsed = bound->heurTimer.elapsed();
		// TODO:: printf("%.2fs initMDP(MDP_RESIDUAL) done\n", bound->elapsed);

		// initialize al array with weak MDP upper bound
		FullObsUBInitializer m;
		m.pomdp = pomdp;
		alpha_vector& alpha = alphas[0];
		copy(m.alpha, alpha);

		FOR (a, pomdp->actions->size()) 
		{
			//al[a].resize(pomdp->YStates->size());
			//nextAl[a].resize(pomdp->YStates->size());
			al[a].resize(pomdp->XStates->size() * pomdp->YStates->size());
			nextAl[a].resize(pomdp->XStates->size() * pomdp->YStates->size());
			m.nextAlphaAction_unfac(al[a], a);
		}

		// iterate FIB update rule to approximate convergence
		do {
			FOR (a, pomdp->actions->size()) {
				DenseVector& beta_a = nextAl[a];

				set_to_zero( beta_a );
				

				FOR (o, pomdp->observations->size()) {
					FOR (i, pomdp->actions->size()) {
						emult_column( tmp, *(*(pomdp->pomdpO))[a], o, al[i] );
						//	  emult_column( tmp, pomdp->pomdpO[a], o, al[i] );
						mult( beta_aoi, tmp, *(*(pomdp->pomdpTtr))[a] );
						//	  mult( beta_aoi, tmp, pomdp->pomdpTtr[a] );
						if (0 == i) {
							beta_ao = beta_aoi;
						} else {
							max_assign( beta_ao, beta_aoi );
						}
					}
					beta_a += beta_ao;
				}

				beta_a *= pomdp->discount;
				copy_from_column( tmp, (*(pomdp->pomdpR)), a );
				//     copy_from_column( tmp, pomdp->pomdpR, a );
				beta_a += tmp;
			}

			maxResidual = 0;
			FOR (a, pomdp->actions->size()) {
				diff = nextAl[a];
				diff -= al[a];
				maxResidual = std::max( maxResidual, diff.norm_inf() );

				al[a] = nextAl[a];
			}

		} while ( maxResidual > targetPrecision );

		if (!getFIBvectors)
		{	DenseVector dalpha;
			FOR (a, pomdp->actions->size()) 
			{
				if (0 == a) 
				{
					dalpha = al[a];
				} 
				else 
				{
					max_assign(dalpha, al[a]);
				}
			}
	
			// post-process: make sure the value for all terminal states
			// is exactly 0, since that is how the ubVal field of terminal
			// nodes is initialized.
			FOR (i, pomdp->XStates->size()*pomdp->YStates->size()) {
	
				// convert i to x and y values
				unsigned int x = (unsigned int) i/(pomdp->YStates->size());
				unsigned int y = i % (pomdp->YStates->size());
	
				if (pomdp->isPOMDPTerminalState[x][y]) {
	
					dalpha(i) = 0.0;
				}
			}
	
			// **** ADDED PRINTOUTS TO SCREEN HERE ******** 6 APRIL 2009 *******
			/*  cout << pomdp->YStates->size() << endl;
			FOR (i, pomdp->YStates->size()) {
			cout << dalpha(i) << " ";
			}
			cout << endl;
			*/
			// write out result
			//  bound->points.clear();
			//  copy(upperBoundBVpair->cornerPoints, dalpha);
	
			// write dalpha entries into dalphas[state_idx] entries
			std::vector<DenseVector> dalphas(pomdp->XStates->size());
	
			FOR (s, pomdp->XStates->size() * pomdp->YStates->size()) {
				// convert i to x and y values
				unsigned int x = (unsigned int) s/(pomdp->YStates->size());
				unsigned int y = s % (pomdp->YStates->size());
	
				if (y==0) dalphas[x].resize(pomdp->YStates->size());
	
				dalphas[x](y) = dalpha(s);
			}
	
			// write out result - do it for each of the BoundsSet
			FOR (state_idx, pomdp->XStates->size()) 
			{
				bound->set[state_idx]->points.clear();
				copy(bound->set[state_idx]->cornerPoints,dalphas[state_idx]);
	
				/*cout << "state_idx  " << state_idx << endl;
				dalphas[state_idx].write(std::cout);
				cout << endl; */
	
			}
		} else {
			actionAlphaByState.resize(pomdp->actions->size());

			FOR (a, pomdp->actions->size()) 
			{
				FOR (i, pomdp->XStates->size()*pomdp->YStates->size()) {
		
					// convert i to x and y values
					unsigned int x = (unsigned int) i/(pomdp->YStates->size());
					unsigned int y = i % (pomdp->YStates->size());
		
					if (pomdp->isPOMDPTerminalState[x][y]) {
		
						al[a](i) = 0.0;
					}

					if (x==0) actionAlphaByState[a].resize(pomdp->XStates->size());
					if (y==0) actionAlphaByState[a][x].resize(pomdp->YStates->size());
					actionAlphaByState[a][x](y) = al[a](i);
				}
			}

		}
	}

	void FastInfUBInitializer::initMDP(double targetPrecision, FullObsUBInitializer& m)
// SYL260809 prevly:	void FastInfUBInitializer::initMDP(double targetPrecision)
	{
		// set alpha to be the mdp upper bound
		// FullObsUBInitializer m;		// SYL260809 commented out
		m.valueIteration(pomdp, targetPrecision); // this puts the MDP vector at m.alphaByState 

		//alphasByState.clear();  

		//alphasByState = m.alphaByState; // SYL260809 commented out

		/* #if DEBUGSYL_160908
		//### check alphasByState contents compared to m.alphaByState contents

		cout << "m.alphaByState" << endl;
		for (unsigned int stateidx=0; stateidx < m.alphaByState.size(); stateidx++)
		{
		m.alphaByState[stateidx].write(std::cout);
		cout << endl;
		}
		cout << "alphasByState" << endl;
		for (unsigned int stateidx=0; stateidx < alphasByState.size(); stateidx++)
		{
		alphasByState[stateidx].write(std::cout);
		cout << endl;
		}
		#endif */



		//std::vector<DenseVector> calphas;
		//copy(calpha, m.alpha);


		//alphas.clear();
		//alphas.push_back(calpha);

		/*
		if (zmdpDebugLevelG >= 1) {
		cout << "initUpperBoundMDP: alpha=" << sparseRep(alphas[0]).c_str() << endl;
		cout << "initUpperBoundMDP: val(b)=" << inner_prod(alphas[0], pomdp->initialBelief) << endl;
		}
		*/
	}

	void FastInfUBInitializer::initFIB(double targetPrecision, bool getFIBvectors)
	{
		// calculates the fast informed bound (Hauskrecht, JAIR 2000)
		//std::vector< DenseVector > al(pomdp->actions->size());
		//std::vector< DenseVector > nextAl(pomdp->actions->size());

		std::vector< std::vector<DenseVector> > al(pomdp->actions->size());
		std::vector< std::vector<DenseVector> > nextAl(pomdp->actions->size());

		DenseVector tmp, tmp2, diff, beta_ao; //beta_aoi, beta_aoiSum
		DenseVector beta_aoXn_unweighted, beta_aoXn;
		double maxResidual;
		//alpha_vector backup;

		FullObsUBInitializer M;  // SYL260809 added 

		initMDP(MDP_RESIDUAL, M); // SYL260809 modified

		//TODO:: bound->elapsed = bound->heurTimer.elapsed();
		//TODO:: printf("%.2fs initMDP(MDP_RESIDUAL) done\n", bound->elapsed);


		// SYL260809 commented out - the MDP upper bound is already in M.alphaByState
		/* 
		// initialize al array with weak MDP upper bound
		FullObsUBInitializer m;
		m.pomdp = pomdp;
		m.alphaByState.resize(pomdp->XStates->size());

		//alpha_vector& alpha = alphas[0];
		//copy(m.alpha, alpha);
		FOR (state_idx, pomdp->XStates->size()) {
			copy(m.alphaByState[state_idx], alphasByState[state_idx]);
		}
		*/

		/* FOR (a, pomdp->actions->size()) {
		al[a].resize(pomdp->YStates->size());
		nextAl[a].resize(pomdp->YStates->size());
		m.nextAlphaAction(al[a], a);
		} */

		FOR (a, pomdp->actions->size()) {
			al[a].resize(pomdp->XStates->size());
			nextAl[a].resize(pomdp->XStates->size());

			M.nextAlphaAction(al[a], a); // SYL260809 prevly: m.nextAlphaAction(al[a], a);
		}

		bool	valid_o_and_Xn; // SYL260809 to allow skipping operation for an observation o, if there are
					// no valid Xn values (for given action a) associated with it
		do {
			// SYL260809 keep track of maximum residual
			maxResidual = 0;

			FOR (a, pomdp->actions->size()) 
			{
				std::vector< DenseVector >& beta_a = nextAl[a];

				FOR (Xc, pomdp->XStates->size()) 
				{
					beta_a[Xc].resize(pomdp->YStates->size());

					FOR (o, pomdp->observations->size()) 
					{
						beta_ao.resize(pomdp->YStates->size());
						valid_o_and_Xn = false; // SYL260809

                                                // only iterate over possible X states
                                                const vector<int>& possibleXns = pomdp->XTrans->getMatrix(a, Xc)->nonEmptyColumns();
					        FOREACH (int, XnIt, possibleXns)
                                                {
                                                    int Xn = *XnIt;
                                                    if (!(pomdp->obsProb->getMatrix(a, Xn)->isColumnEmpty(o)))
                                                    {
                                                        valid_o_and_Xn = true;	 // SYL260809

                                                        FOR (i, pomdp->actions->size()) 
                                                        {
                                                            emult_column( tmp, *pomdp->obsProb->getMatrix(a, Xn), o, al[i][Xn] );
                                                            mult( tmp2, *pomdp->YTrans->getMatrix(a, Xc, Xn), tmp); // SYL270809
                                                            // SYL270809 mult(vector, matrix, vector) is faster than mult(vector, vector, matrix)
                                                            //mult( tmp2, tmp, *pomdp->XYTrans->getMatrixTr(a, Xc) );

                                                            if (0 == i) {
                                                                beta_aoXn_unweighted = tmp2;
                                                            } else {
                                                                max_assign( beta_aoXn_unweighted, tmp2 );
                                                            }

                                                        }

                                                        emult_column(beta_aoXn, *pomdp->XTrans->getMatrix(a, Xc), Xn, beta_aoXn_unweighted);
                                                        beta_ao += beta_aoXn;
                                                    }
                                                }
									// SYL260809
						if (valid_o_and_Xn)	// false means there's no valid Xn (given the 
									//action and Xc) for this observation o
							beta_a[Xc] += beta_ao; //beta_ao;

					}

					beta_a[Xc] *= pomdp->discount;
					
					copy_from_column( tmp, *pomdp->rewards->getMatrix(Xc), a );
					beta_a[Xc] += tmp;

					// SYL260809 keep track of maximum residual
					diff = beta_a[Xc];
					diff -= al[a][Xc];
					maxResidual = std::max( maxResidual, diff.norm_inf() );
				}
			}

			// SYL260809  assign the FIB vectors for next iteration
			al = nextAl;

			/*
			if (zmdpDebugLevelG >= 1) {
			cout << ".";
			cout.flush();
			}
			*/

		} while ( maxResidual > targetPrecision );

		//cout << "targetPrecision : " << targetPrecision << endl;


		/*
		if (zmdpDebugLevelG >= 1) {
		cout << endl;
		}
		*/
#if DEBUGSYL_290908
		cout << "targetPrecision : " << targetPrecision << endl;
		cout << "maxResidual : " << maxResidual << " iterationCount : " << iterationCount << endl;
#endif

#if DEBUGSYL_160908
		cout << "targetPrecision : " << targetPrecision << endl;
		cout << "maxResidual : " << maxResidual << " iterationCount : " << iterationCount << endl;
		cout << "After iteration, al " << endl;
		FOR (a, pomdp->actions->size()) {
			cout << "a : " << a << endl;

			for (unsigned int stateidx=0; stateidx < alphasByState.size(); stateidx++)
			{
				cout << "stateidx : " << stateidx << endl;
				al[a][stateidx].write(std::cout);
				cout << endl;
			}
		}
#endif 

		if (!getFIBvectors) {
			// SYL260809 we only need one dalpha of size YStates.size at a time
			DenseVector dalpha;
			//std::vector<DenseVector> dalpha(pomdp->XStates->size());
			FOR (state_idx, pomdp->XStates->size()) {
	
				FOR (a, pomdp->actions->size()) {
					if (0 == a) {
						dalpha = al[a][state_idx];
					} else {
						max_assign(dalpha, al[a][state_idx]);
					}
				}
	
				// at this point, the vector at dalpha[state_idx] contains the highest value, across actions
	
				// post-process: make sure the value for all terminal states
				// is exactly 0, since that is how the ubVal field of terminal
				// nodes is initialized.
				FOR (i, pomdp->YStates->size()) {
					if (pomdp->isPOMDPTerminalState[state_idx][i]) {
						dalpha(i) = 0.0;
					}
				}
	
				// at this point, the vector at dalpha[state_idx] has taken into account terminal state
				bound->set[state_idx]->points.clear();
				copy(bound->set[state_idx]->cornerPoints,dalpha);
	
			} 

		} else {  // output one vector for each action

			FOR (state_idx, pomdp->XStates->size()) {
	
				FOR (a, pomdp->actions->size()) {	
				// post-process: make sure the value for all terminal states
				// is exactly 0
					FOR (i, pomdp->YStates->size()) {
						if (pomdp->isPOMDPTerminalState[state_idx][i]) {
							al[a][state_idx](i) = 0.0;
						}
					}
				}
	
			} 

			actionAlphaByState = al;

		}
	}

}; // namespace zmdp


