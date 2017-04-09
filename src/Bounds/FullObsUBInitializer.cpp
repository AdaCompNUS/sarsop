/** 
* Part of the this code is derived from ZMDP: http://www.cs.cmu.edu/~trey/zmdp/
* ZMDP is released under Apache License 2.0
* The rest of the code is released under GPL v2 
*/


#include <assert.h>
#include <stdlib.h>
#ifdef _MSC_VER
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <math.h>
#include <cfloat>

#include <algorithm>
#include <iostream>

#include "FullObsUBInitializer.h"


using namespace std;
//using namespace MatrixUtils;

//#define DEBUGSYL_170908 1

namespace momdp {

	void FullObsUBInitializer::nextAlphaAction_unfac(DenseVector& result, int a) 
	{
		DenseVector R_xa;
		mult(result, *(*(pomdp->pomdpT))[a], alpha); // SYL040909 prevly mult( result, alpha, *(*(pomdp->pomdpTtr))[a] ); The current function call mult(result, matrix, vector) seems faster
		result *= pomdp->discount;
		copy_from_column( R_xa, *(pomdp->pomdpR), a );
		result += R_xa;
	}

	double FullObsUBInitializer::valueIterationOneStep_unfac(void) 
	{
		DenseVector nextAlpha(pomdp->XStates->size() * pomdp->YStates->size());
		DenseVector naa(pomdp->XStates->size() * pomdp->YStates->size());
		DenseVector tmp;
		double maxResidual;

		nextAlphaAction_unfac(nextAlpha,0);

		FOR (a, pomdp->actions->size()) 
		{
			nextAlphaAction_unfac(naa,a);

			FOR (s, pomdp->XStates->size() * pomdp->YStates->size()) 
			{
				if (naa(s) > nextAlpha(s))
				{
					nextAlpha(s) = naa(s);
				}
			}
		}

		tmp.resize( alpha.size() );
		tmp = alpha;
		tmp -= nextAlpha;

		maxResidual = tmp.norm_inf();
		alpha = nextAlpha;

		return maxResidual;
	}

	void FullObsUBInitializer::valueIteration_unfac( SharedPointer<MOMDP> _pomdp, double eps) 
	{
		pomdp = _pomdp;


		alpha.resize(pomdp->XStates->size() * pomdp->YStates->size());
		//alpha.resize(pomdp->getBeliefSize());
		set_to_zero(alpha);


		double residual;
		FOR (i, MDP_MAX_ITERS) 
		{
			residual = valueIterationOneStep_unfac();
			
			if (residual < eps) 
			{
				return;
			}
		}
		cout << endl
			<< "failed to reach desired eps of " << eps << " after "
			<< MDP_MAX_ITERS << " iterations" << endl;
		cout << "residual = " << residual << endl;
	}


	void FullObsUBInitializer::QNextAlphaAction_unfac(DenseVector& result, int a) 
	{
		DenseVector R_xa;
		mult( result, actionAlphas[a], *(*(pomdp->pomdpTtr))[a] );
		result *= pomdp->discount;
		copy_from_column( R_xa, *(pomdp->pomdpR), a );
		result += R_xa;
	}

	double FullObsUBInitializer::QValueIterationOneStep_unfac(void) 
	{
		DenseVector nextAlpha(pomdp->XStates->size() * pomdp->YStates->size());
		DenseVector naa(pomdp->XStates->size() * pomdp->YStates->size());
		DenseVector tmp;
		double maxResidual = -DBL_MAX;

		FOR (a, pomdp->actions->size()) 
		{
			DenseVector nextAlpha(actionAlphas[a].size());
			DenseVector tmp;
			tmp = actionAlphas[a];
			QNextAlphaAction_unfac(nextAlpha,a);
			tmp -= nextAlpha;
			double residual = tmp.norm_inf();
			if(residual > maxResidual)
			{
				maxResidual = residual;
			}
			actionAlphas[a] = nextAlpha;
		}

		return maxResidual;
	}

	void FullObsUBInitializer::QMDPSolution_unfac( SharedPointer<MOMDP> _pomdp, double eps) 
	{
		pomdp = _pomdp;
		actionAlphas.resize(pomdp->actions->size());

		valueIteration_unfac(pomdp, eps); // this will put the result of MDP value iteration in alpha.


		FOR(a, pomdp->actions->size())
		{
			actionAlphas[a].resize(pomdp->XStates->size() * pomdp->YStates->size());
			nextAlphaAction_unfac(actionAlphas[a],a); // onestep backup on alpha with action "a"

		}

	}


	void FullObsUBInitializer::QValueIteration_unfac( SharedPointer<MOMDP> _pomdp, double eps) 
	{
		pomdp = _pomdp;
		actionAlphas.resize(pomdp->actions->size());
		FOR(a, pomdp->actions->size())
		{
			actionAlphas[a].resize(pomdp->XStates->size() * pomdp->YStates->size());
			set_to_zero(actionAlphas[a]);
		}

		double residual;
		FOR (i, MDP_MAX_ITERS) 
		{
			residual = QValueIterationOneStep_unfac();
			if (residual < eps) 
			{
				return;
			}
		}
		cout << endl
			<< "failed to reach desired eps of " << eps << " after "
			<< MDP_MAX_ITERS << " iterations" << endl;
		cout << "residual = " << residual << endl;
	}


	void FullObsUBInitializer::nextAlphaAction(std::vector<DenseVector>& resultByState, int a) 
	{

		DenseVector resultThisState(pomdp->YStates->size()), resultSum(pomdp->YStates->size());
		DenseVector tmp(pomdp->YStates->size());
		DenseVector R_xa;

		FOR (state_idx, pomdp->XStates->size()) 
		{
			set_to_zero(resultSum); 
			// total expected next value
                        // only iterate over possible X states
                        const vector<int>& possibleXns = pomdp->XTrans->getMatrix(a, state_idx)->nonEmptyColumns();
                        FOREACH (int, XnIt, possibleXns)
                        {
                            int Xn = *XnIt;
                            // expected next value for Xn
                            mult( tmp, *pomdp->YTrans->getMatrix(a, state_idx, Xn), alphaByState[Xn]);
                            // SYL260809 prevly is as below. The current function called, mult(result, matrix, vector), seems faster
                            //mult( tmp, alphaByState[Xn], *pomdp->XYTrans->matrixTr[a][state_idx] );
                            emult_column( resultThisState, *pomdp->XTrans->getMatrix(a, state_idx), Xn, tmp );
                            resultSum += resultThisState;
                        }

			resultSum *= pomdp->discount;
			copy_from_column( R_xa, *pomdp->rewards->getMatrix(state_idx), a );
			resultSum += R_xa;
			resultByState[state_idx] = resultSum;
		} 
	}


	double FullObsUBInitializer::valueIterationOneStep(void) 
	{
		//DenseVector nextAlpha(pomdp->getBeliefSize()), naa(pomdp->getBeliefSize());
		DenseVector tmp; //, tmpzero(pomdp->YStates->size());
		double maxResidual; // = 0;

		std::vector<DenseVector> nextAlpha(pomdp->XStates->size()), naa(pomdp->XStates->size());

		nextAlphaAction(nextAlpha,0);
		//FOR (a, pomdp->actions->size()) {
		for (unsigned int a=1; a < pomdp->actions->size(); a++) 
		{
			nextAlphaAction(naa,a);		
			FOR (state_idx, pomdp->XStates->size()) 
			{
				FOR (s, pomdp->getBeliefSize()) 
				{
					if (naa[state_idx](s) > nextAlpha[state_idx](s)) 
					{
						nextAlpha[state_idx](s) = naa[state_idx](s);
					}
				}
			}
		}

		FOR (state_idx, pomdp->XStates->size()) 
		{
			//tmp.resize( alphaByState[state_idx].size() );
			tmp = alphaByState[state_idx];
			tmp -= nextAlpha[state_idx];

			if (state_idx == 0)
				maxResidual = tmp.norm_inf();
			else {
				if (tmp.norm_inf() >  maxResidual)
					maxResidual =tmp.norm_inf();
			}

		}

		alphaByState = nextAlpha;

		return maxResidual;
	}

	void FullObsUBInitializer::valueIteration(SharedPointer<MOMDP> _pomdp, double eps) 
	{
		pomdp = _pomdp;

		alphaByState.resize(pomdp->XStates->size());


		FOR (state_idx, pomdp->XStates->size()) 
		{
			//cout << "pomdp->getBeliefSize() " << pomdp->getBeliefSize() << endl;
			alphaByState[state_idx].resize(pomdp->getBeliefSize());
			//set_to_zero(alphaByState[state_idx]);
		}

		double residual;
		FOR (i, MDP_MAX_ITERS) 
		{
			residual = valueIterationOneStep();
			if (residual < eps) 
			{
				return;
			}
		}
		cout << endl
			<< "failed to reach desired eps of " << eps << " after "
			<< MDP_MAX_ITERS << " iterations" << endl;
		cout << "residual = " << residual << endl;
	}
	void FullObsUBInitializer::QNextAlphaAction(std::vector<DenseVector>& resultByState, int a) 
	{

		DenseVector resultThisState(pomdp->YStates->size()), resultSum(pomdp->YStates->size());
		DenseVector tmp(pomdp->YStates->size());
		DenseVector R_xa;

		FOR (state_idx, pomdp->XStates->size()) 
		{
			set_to_zero(resultSum); 
			// total expected next value
                        // only iterate over possible X states
                        const vector<int>& possibleXns = pomdp->XTrans->getMatrix(a, state_idx)->nonEmptyColumns();
                        FOREACH (int, XnIt, possibleXns)
                        {
                            int Xn = *XnIt;
                            // expected next value for Xn
                            mult( tmp, actionAlphaByState[a][Xn], *pomdp->YTrans->getMatrixTr(a, state_idx, Xn) );
                            emult_column( resultThisState, *pomdp->XTrans->getMatrix(a, state_idx), Xn, tmp );
                            resultSum += resultThisState;

                        }

			resultSum *= pomdp->discount;
			copy_from_column( R_xa, *pomdp->rewards->getMatrix(state_idx), a );
			resultSum += R_xa;
			resultByState[state_idx] = resultSum;
		} 
	}

	double FullObsUBInitializer::QValueIterationOneStep(void) 
	{
		//DenseVector nextAlpha(pomdp->getBeliefSize()), naa(pomdp->getBeliefSize());
		DenseVector tmp; //, tmpzero(pomdp->YStates->size());
		double maxResidual = - DBL_MAX;

		FOR (a, pomdp->actions->size())
		{
			vector<DenseVector> nextAlpha(pomdp->XStates->size());
			QNextAlphaAction(nextAlpha,a);


			FOR (state_idx, pomdp->XStates->size()) 
			{
				tmp = actionAlphaByState[a][state_idx];
				tmp -= nextAlpha[state_idx];

				if (tmp.norm_inf() >  maxResidual)
				{
					maxResidual =tmp.norm_inf();
				}

				// 16092008 changed, prevly was probably too stringent
				//maxResidual += norm_inf(tmp);		
			}

			actionAlphaByState[a] = nextAlpha;
		}
		return maxResidual;
	}

	void FullObsUBInitializer::QMDPSolution(SharedPointer<MOMDP> _pomdp, double eps) 
	{
		pomdp = _pomdp;

		actionAlphaByState.resize(pomdp->actions->size());

		valueIteration(pomdp, eps); // the MDP solution is now in alphaByState

		FOR (a, pomdp->actions->size()) {
			actionAlphaByState[a].resize(pomdp->XStates->size());
			nextAlphaAction(actionAlphaByState[a], a);  // nextAlphaAction() does onestep backup on alphaByState, with action "a".
		}
	}

	void FullObsUBInitializer::QValueIteration(SharedPointer<MOMDP> _pomdp, double eps) 
	{
		pomdp = _pomdp;

		actionAlphaByState.resize(pomdp->actions->size());
		FOR(a, pomdp->actions->size())
		{
			actionAlphaByState[a].resize(pomdp->XStates->size());
			FOR (state_idx, pomdp->XStates->size()) 
			{
				actionAlphaByState[a][state_idx].resize(pomdp->getBeliefSize());
				set_to_zero(actionAlphaByState[a][state_idx]);
			}

		} 

		double residual;
		FOR (i, MDP_MAX_ITERS) 
		{
			residual = QValueIterationOneStep();
			if (residual < eps) 
			{
				return;
			}
		}
		cout << endl
			<< "failed to reach desired eps of " << eps << " after "
			<< MDP_MAX_ITERS << " iterations" << endl;
		cout << "residual = " << residual << endl; 
	}

	void FullObsUBInitializer::FacPostProcessing(vector<alpha_vector>& alphasByState)
	{
		FOR (state_idx, pomdp->XStates->size()) 
		{
			FOR (i, pomdp->YStates->size()) 
			{
				if (pomdp->isPOMDPTerminalState[state_idx][i]) 
				{
					alphasByState[state_idx](i) = 0.0;
				}
			}
		}
	}
	void FullObsUBInitializer::UnfacPostProcessing(DenseVector& calpha, vector<alpha_vector>& alphasByState)
	{
		// post-process: make sure the value for all terminal states
		// is exactly 0, since that is how the ubVal field of terminal
		// nodes is initialized.
		int numXState = pomdp->XStates->size();
		int numYState = pomdp->YStates->size();
		FOR (i,numXState *numYState)
		{

			// convert i to x and y values
			unsigned int x = (unsigned int) i/numYState;
			unsigned int y = i % numYState;

			if (pomdp->isPOMDPTerminalState[x][y]) 
			{
				calpha(i) = 0.0;
			}
		}
		// write dalpha entries into dalphas[state_idx] entries
		FOR (x, numXState)
		{
			alphasByState[x].resize(numYState);
		}
		FOR (s, numXState * pomdp->YStates->size()) 
		{
			// convert i to x and y values
			unsigned int x = (unsigned int) s/numYState;
			unsigned int y = s % numYState;
			alphasByState[x](y) = calpha(s);
		}

	}


}; // namespace zmdp

