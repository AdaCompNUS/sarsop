/** 
 * The code is released under GPL v2 
 */
/************************************************************************
File: SampleBP.cc
Date: 06/06/2007 
Author: rn
*************************************************************************/
#include "SampleBP.h"
#include "SARSOP.h"

#include "GlobalResource.h"

//#define  DEBUGSYL_310708 1
//#define  DEBUGSYL_120808 1

namespace momdp
{
	//constructor
	void SampleBP::setup(SharedPointer<MOMDP> _problem, SARSOP* _solver)
	{
		Sample::setup(_solver, _problem, &(_solver->beliefCacheSet), _solver->beliefForest);

		problem = _problem;
		solver = _solver;
		srand( time(NULL) );
		// beliefCache = &(BeliefCache::getCache ()); // removed for factored, now accessed through Bounds class
		

		numTrials = 0;
		numSubOptimal = 0;		//xan-edit
		numBinProceed = 0;//for Bin Heuristic

		// this section modified for running parallel trials
		depthArr.resize(problem->initialBeliefX->size());
		trialTargetPrecisionArr.resize(problem->initialBeliefX->size());
		newTrialFlagArr.resize(problem->initialBeliefX->size());
		priorityQueueArr.resize(problem->initialBeliefX->size());
		nextNodeTargetUbArr.resize(problem->initialBeliefX->size());
		nextNodeTargetLbArr.resize(problem->initialBeliefX->size());
		FOR(r, problem->initialBeliefX->size()) 
		{
			double rprob = (*(problem->initialBeliefX))(r);
			if (rprob > OBS_IS_ZERO_EPS) {
				depthArr[r] = 0;
				trialTargetPrecisionArr[r] = -1;
				newTrialFlagArr[r]  = 0; //ADD SYLTAG
			} else {
				depthArr[r] = -10;
				trialTargetPrecisionArr[r] = -10;
				newTrialFlagArr[r]  = -10; //ADD SYLTAG
			}
		}
	}
	SampleBP::SampleBP ()
	{
	}

	void SampleBP::setRandomization(bool newFlag)
	{
		doRandomization = newFlag;
	}
	//first level method
	//Function: sample
	//Functionality: 
	//    decide which are the next beliefs to be sampled, and
	//    return them as a list, at the same time add them into the
	//    reachable structure
	//Parameters:
	//    row: the current belief's row index in beliefCache
	//Returns:
	//    the list of belief indices in beliefCache which will do backups next
	list < cacherow_stval >SampleBP::sample (cacherow_stval currIndexRow, unsigned int currentRoot)
	{
		isRoot = false;
		bm->updateNode(currIndexRow); //for Bin Heuristic  // prevly, bm->updateNode(row);//for Bin Heuristic 
		list < cacherow_stval >sampledBeliefs; // prevly, list < int >sampledBeliefs;
		double excessUncertainty;
		double lbVal, ubVal;

		
		lbVal = solver->beliefCacheSet[currIndexRow.sval]->getRow(currIndexRow.row)->LB;
		ubVal = solver->beliefCacheSet[currIndexRow.sval]->getRow(currIndexRow.row)->UB;
		BeliefTreeNode & currentNode = *(solver->beliefCacheSet[currIndexRow.sval]->getRow(currIndexRow.row)->REACHABLE);

		DEBUG_TRACE( cout << "SampleBP::sample ub" << ubVal << " lb " << lbVal << endl; );
		DEBUG_TRACE( cout << "SampleBP::sample currentNode row: " << currentNode.cacheIndex.row << " sval " << currentNode.cacheIndex.sval << endl; );

		//reset depth and precision parameters when it is a new trial
		if (trialTargetPrecisionArr[currentRoot] == -1)//need for hsvi and fixed precision bp
		//if (trialTargetPrecision == -1)//need for hsvi and fixed precision bp
		{
			//ADD SYLTAG
			bool rootFound = false;
			
			FOR (r, solver->beliefForest->getGlobalRootNumSampleroots()) // solver->beliefForest is a data member of Sample class
			{
				SampleRootEdge* eR = solver->beliefForest->sampleRootEdges[r];
				if (NULL != eR) {
					BeliefTreeNode & sn = *eR->sampleRoot;
					if (&currentNode == &sn) rootFound  = true;
				}
			}

			//MOD SYLTAG
			if (!rootFound)
			//if (&currentNode != root) // prevly if (row != 0)	// root is a data member of Sample class
			{
				printf ("error: new trial not starting from the root\n");
			}

			//trialTargetPrecision = (ubVal - lbVal) * problem->getDiscount ();
			//cout << "GlobalResource::getInstance()->solverParams->BP_IMPROVEMENT_CONSTANT " << GlobalResource::getInstance()->solverParams->BP_IMPROVEMENT_CONSTANT << endl;
			
			trialTargetPrecisionArr[currentRoot] = (ubVal - lbVal) *  solver->solverParams->BP_IMPROVEMENT_CONSTANT;//not 
			//trialTargetPrecision = (ubVal - lbVal) * GlobalResource::getInstance()->solverParams->BP_IMPROVEMENT_CONSTANT;//not in use for fixed targetPrecision 
			//empty the priority Queue
			priorityQueueArr[currentRoot].clear ();
			isRoot = true;
			nextNodeTargetUbArr[currentRoot] = lbVal;
		}

		double finalExcess = ubVal - lbVal - (trialTargetPrecisionArr[currentRoot] * 0.5 * pow (problem->getDiscount (), -depthArr[currentRoot]));

		DEBUG_TRACE( cout << "SampleBP::sample finalExcess " << finalExcess << endl; );

		if ( (finalExcess) <= 0 )//if final precision reached
		{
			numTrials++;
			trialTargetPrecisionArr[currentRoot]  = -1;
			depthArr[currentRoot] = 0;
			logOcc = log (1.0);
			newTrialFlagArr[currentRoot] = 1; //ADD SYLTAG

			return priorityQueueArr[currentRoot];	//at the end of each trial, the entire path is returned
		}
		else
		{
			//initialize checking variables----------------------
			double expectedError = trialTargetPrecisionArr[currentRoot]  * pow (problem->getDiscount (), -depthArr[currentRoot]);
			excessUncertainty = ubVal - lbVal - expectedError;
			double curTargetUb = max (nextNodeTargetUbArr[currentRoot], lbVal + expectedError);

			BPUpdateResult r;

			DEBUG_TRACE( cout << "SampleBP::sample expectedError " << expectedError << endl; );
			DEBUG_TRACE( cout << "SampleBP::sample excessUncertainty " << excessUncertainty << endl; );
			
			// choose to do randomization or not
			if(doRandomization)
			{
				r.maxUBAction = chooseAction(currentNode);
				//cout << "Random Choose action" << endl;
			}
			else
			{
				r.maxUBAction = solver->upperBoundSet->set[currIndexRow.sval]->dataTable->get(currIndexRow.row).UB_ACTION;
			}
			
			DEBUG_TRACE( cout << "SampleBP::sample r.maxUBAction " << r.maxUBAction << endl; );

			r.maxUBVal = ubVal;
			getMaxExcessUncOutcome (currentNode, r, currentRoot);

			//check whether target upper bound is reached by the newly sampled node
			int i = r.maxUBAction;
			int Xn = r.maxExcessUncStateOutcome;
			int j = r.maxExcessUncOutcome;

			cacherow_stval newIndexRow = currentNode.getNextState (i,j, Xn).cacheIndex;
	
			DEBUG_TRACE( cout << "SampleBP::sample int i = r.maxUBAction; " << r.maxUBAction << endl; );
			DEBUG_TRACE( cout << "SampleBP::sample int Xn = r.maxExcessUncStateOutcome; " << Xn << endl; );
			DEBUG_TRACE( cout << "SampleBP::sample int j = r.maxExcessUncOutcome; " << j << endl; );
			DEBUG_TRACE( cout << "SampleBP::sample newIndexRow row " << newIndexRow.row <<  " sval " << newIndexRow.sval << endl; );


			// 26092008 added
			// if child node is exactly the same as parent node, and there is no randomization,
			// end the trial here 
			// HOWEVER note that , since the parent node is a child node of itself, just doing repeated backups COULD
			// improve bounds at the same node!
/*			if ( (!doRandomization) && (newIndexRow.sval == currIndexRow.sval) && (newIndexRow.row == currIndexRow.row))
			{
				numTrials++;
				trialTargetPrecision = -1;	//real value computed later when needed
				depth = 0;
				logOcc = log (1.0);
				newTrialFlag = 1; //ADD SYLTAG
#if DEBUGSYL_260908
cout << "End of one trial due to childnode == parentnode" << endl;
#endif
				return priorityQueue;	//at the end of each trial, the entire path is returned
			} 
*/

			// int newRow = currentNode.getNextState (i,j).cacheIndex;
			double futureValue = 0;
			const BeliefTreeQEntry & Qa = currentNode.Q[i];
			bool proceed = false;
			//compute expectation ub
			FOR (Xval, Qa.getNumStateOutcomes())
			{
				const BeliefTreeObsState* QaXval = Qa.stateOutcomes[Xval];
				//const BeliefTreeObsState & QaXval = Qa.stateOutcomes[Xval];
			   if (NULL != QaXval ) {
				FOR (o, QaXval->getNumOutcomes ())
				//FOR (o, QaXval.getNumOutcomes ())
				{
					BeliefTreeEdge *e = QaXval->outcomes[o];
					//BeliefTreeEdge *e = QaXval.outcomes[o];
					if (NULL != e && ((o != j) || (Xval != Xn)   ))
					{
						// 26092008 corrected multiplication factor, prevly: e->obsProb
						// 061008 changed calculation for obsProb
						futureValue += e->obsProb * solver->beliefCacheSet[e->nextState->cacheIndex.sval]->getRow(e->nextState->cacheIndex.row)->UB;
						//futureValue += e->obsProb * QaXval->obsStateProb * bounds->boundsSet[e->nextState->cacheIndex.sval]->beliefCache->getRow(e->nextState->cacheIndex.row)->UB;
					}
				}
			   }
			}

			/* FOR (o, Qa.getNumOutcomes ())
			{
				BeliefTreeEdge *e = Qa.outcomes[o];
				if (NULL != e && o != j)
				{
					futureValue += e->obsProb * beliefCache->getRow(e->nextState->cacheIndex)->UB;
				}
			} */

			//end initializing variables----------------------

			if ( CompareIfLowerBoundImprovesAction(i, j, Xn, currentNode, currentRoot) )//if bin value improves lb
			//if ( CompareIfLowerBoundImprovesAction(i, j, currentNode) )//if bin value improves lb
			{
				DEBUG_TRACE ( cout << "proceed1" << endl; );
				numBinProceed++;
				proceed = true;
			}
			else
			{
				//if termination condition true, return root and start a new trial
				if ( (excessUncertainty) <= 0)
				{
					DEBUG_TRACE ( cout << "proceed2" << endl; );
					numTrials++;
					trialTargetPrecisionArr[currentRoot] = -1;	//real value computed later when needed
					depthArr[currentRoot] = 0;
					logOcc = log (1.0);
					newTrialFlagArr[currentRoot] = 1; //ADD SYLTAG
#if DEBUGSYL_260908
cout << "End of one trial due to ( (excessUncertainty) <= 0)" << endl;
#endif

					return priorityQueueArr[currentRoot];	//at the end of each trial, the entire path is returned
				} 
				else
				{
					if ( (currentNode.Q[i].ubVal == CB_QVAL_UNDEFINED || currentNode.Q[i].ubVal > curTargetUb))
					{
						DEBUG_TRACE ( cout << "proceed3" << endl; );
						proceed = true;
					}
					else
					{	//targetUpperBound is not reached for this action
						DEBUG_TRACE ( cout << "proceed4" << endl; );
						numSubOptimal++;//subOptimal = true;
					}//end bp check
				}//end excess uncertainty check
			}//end compare lower bound improves action
			
			
			if ( proceed )
			{
				DEBUG_TRACE ( cout << "proceeded" << endl; );
#if DEBUGSYL_260908
cout << "In sample(), parent.sval : " << currIndexRow.sval << " .row : " << currIndexRow.row ;
cout << " | action : " << i << " observation : " << j ;
cout << " | child.sval : " << newIndexRow.sval << ".row " << newIndexRow.row << endl;
#endif

				//double curUb = bounds->upperBounds->getValue (currentNode.getNextState (i, j).s);
				double curUb = solver->upperBoundSet->getValue(currentNode.getNextState (i, j, Xn).s);

				//double curUb = bounds->upperBoundBVpair->getValue (currentNode.getNextState (i, j).s);
				solver->beliefCacheSet[newIndexRow.sval]->getRow(newIndexRow.row)->UB = curUb; // beliefCache->getRow(newRow)->UB = curUb;
				// 061008 changed calculation for obsProb
				nextNodeTargetUbArr[currentRoot] = ((curTargetUb - currentNode.Q[i].immediateReward) / problem->getDiscount() - futureValue) / (Qa.stateOutcomes[Xn]->outcomes[j]->obsProb) ;
				//nextNodeTargetUb = ((curTargetUb - currentNode.Q[i].immediateReward) / problem->getDiscount() - futureValue) / (Qa.stateOutcomes[Xn]->outcomes[j]->obsProb * Qa.stateOutcomes[Xn]->obsStateProb) ;
				// the above takes the place of this
				//nextNodeTargetUb = ((curTargetUb - currentNode.Q[i].immediateReward) / problem->getDiscount() - futureValue) / Qa.stateOutcomes[Xn]->outcomes[j]->obsProb;

				//nextNodeTargetUb = ((curTargetUb - currentNode.Q[i].immediateReward) / problem->getDiscount() - futureValue) / Qa.outcomes[j]->obsProb;
				assert (-1 != r.maxExcessUncOutcome);
				sampledBeliefs.push_back (newIndexRow);
				samplePrepare (sampledBeliefs.back ());
//TODO				dump(currIndexRow, newIndexRow);
				priorityQueueArr[currentRoot].push_front (currIndexRow);
				depthArr[currentRoot]++;

				return sampledBeliefs;
			}
			else
			{
				numTrials++;
				trialTargetPrecisionArr[currentRoot] = -1;	//real value computed later when needed
				depthArr[currentRoot] = 0;
				logOcc = log (1.0);
				newTrialFlagArr[currentRoot] = 1; //ADD SYLTAG

#if DEBUGSYL_260908
cout << "End of one trial due to !( (currentNode.Q[i].ubVal == CB_QVAL_UNDEFINED || currentNode.Q[i].ubVal > curTargetUb))" << endl;
#endif


				return priorityQueueArr[currentRoot];	//at the end of each trial, the entire path is returned
			}
		}//end final precision check
	}

	//third level method
	//Function: getMaxExecessUncOutcome
	//Functionality:
	//    update the maxExcessUnc and maxExcessUncOutcome for r
	//Parameters:
	//    cn:     the node at which updates needs to be done
	//    r:      the result of which to be updated
	//Returns:
	//    NA
	//Called by: sample()
	void SampleBP::getMaxExcessUncOutcome (BeliefTreeNode & cn, BPUpdateResult & r, unsigned int currentRoot) const
	{
		r.maxExcessUnc = -99e+20;
		r.maxExcessUncOutcome = -1;
		r.maxExcessUncStateOutcome = -1;
		double width;
		double lbVal, ubVal;
		BeliefTreeQEntry & Qa = cn.Q[r.maxUBAction];
		FOR (x, Qa.getNumStateOutcomes() )
		{
			BeliefTreeObsState* Qax = Qa.stateOutcomes[x];
			//BeliefTreeObsState & Qax = Qa.stateOutcomes[x];
		  if(NULL != Qax) {
			FOR (o, Qax->getNumOutcomes ())
			//FOR (o, Qax.getNumOutcomes ())
			{
				BeliefTreeEdge *e = Qax->outcomes[o];
				//BeliefTreeEdge *e = Qax.outcomes[o];
				if (NULL != e)
				{
					BeliefTreeNode & sn = *e->nextState;
					lbVal =	solver->beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->LB;
					ubVal =	solver->beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->UB;
					// 26092008 corrected multiplication factor, prevly: e->obsProb
					// 061008 changed calculation for obsProb
					width =	e->obsProb * (ubVal - lbVal - trialTargetPrecisionArr[currentRoot] * pow (problem->getDiscount (), -(depthArr[currentRoot] + 1)));//targetPrecision VS trialTargetPrecision ??
					//width =	e->obsProb * Qax->obsStateProb * (ubVal - lbVal - trialTargetPrecision * pow (problem->getDiscount (), -(depth + 1)));//targetPrecision VS trialTargetPrecision ??
					if (width > r.maxExcessUnc)
					{
						r.maxExcessUnc = width;
						r.maxExcessUncOutcome = o;
						r.maxExcessUncStateOutcome = x;
					}
				}
			}
		   }
		}

		if(doRandomization)
		{
			//randomly select observation

			int numObservations = problem->observations->size();
			int numStatesObs = problem->XStates->size();

			int * xstates = (int *)malloc(sizeof(int)*numObservations*numStatesObs);
			int * observations = (int *)malloc(sizeof(int)*numObservations*numStatesObs);
			double * widths = (double *)malloc(sizeof(double)*numObservations*numStatesObs);
			double max_excess = r.maxExcessUnc;
			int index = 0;

			FOR (x, Qa.getNumStateOutcomes() )
			{
				BeliefTreeObsState* Qax = Qa.stateOutcomes[x];
				//BeliefTreeObsState & Qax = Qa.stateOutcomes[x];
			   if (NULL != Qax) {
				FOR (o, Qax->getNumOutcomes()) {
				//FOR (o, Qax.getNumOutcomes()) {
					BeliefTreeEdge* e = Qax->outcomes[o];
					//BeliefTreeEdge* e = Qax.outcomes[o];
					if (NULL != e) {
						BeliefTreeNode& sn = *e->nextState;
					lbVal =	solver->beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->LB;
					ubVal =	solver->beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->UB;
					// 26092008 corrected multiplication factor, prevly: e->obsProb
					// 061008 changed calculation for obsProb
					width =	e->obsProb * (ubVal - lbVal - trialTargetPrecisionArr[currentRoot] * pow (problem->getDiscount (), -(depthArr[currentRoot] + 1)));//targetPrecision VS trialTargetPrecision ??
					//width =	e->obsProb * Qax->obsStateProb * (ubVal - lbVal - trialTargetPrecision * pow (problem->getDiscount (), -(depth + 1)));//targetPrecision VS trialTargetPrecision ??
						if (width >= (max_excess - 0.5*1e-9) ) {
							observations[index] = o;
							xstates[index] = x;
							widths[index] = width;
							index++;
						}
					}
				}
			    }
			}
			int randVal = rand() % index;
			r.maxExcessUnc = widths[randVal];
			r.maxExcessUncOutcome = observations[randVal];
			r.maxExcessUncStateOutcome = xstates[randVal];
			free(observations);
			free(xstates);
			free(widths);

		}
	}//end method: getMaxExcessUncOutcome

	void SampleBP::setBinManager (BinManagerSet * _bm)
	{
		bm = _bm;
	}

	bool SampleBP::CompareIfLowerBoundImprovesAction(int action, int observation, int xstate, BeliefTreeNode & currentNode,  unsigned int currentRoot)
	{
		int maxAction;
		double maxValue = -99e10;
		FOR (a, problem->getNumActions()) {//Find the best action and the corresponding value
			if (currentNode.Q[a].lbVal > maxValue) {
				maxValue = currentNode.Q[a].lbVal;
				maxAction = a;
			}
		}

		double value = 0;
		double sum_o = 0;
		//outcome_prob_vector opv;

		// basically, we want to cycle through all valid children of Q[action], except the chosen one (action, xstate, observation)
		// and sum up their LB values, multiplied by P(o,x'|x,b,a)
		const BeliefTreeQEntry & Qa = currentNode.Q[action];
		FOR (x, Qa.getNumStateOutcomes())
		{
			const BeliefTreeObsState* Qax = Qa.stateOutcomes[x];
			if (NULL != Qax ) {
				FOR (o, Qax->getNumOutcomes ())
				{
					BeliefTreeEdge *e = Qax->outcomes[o];

					if ( NULL != e && ((o != (unsigned int)observation) || (x != xstate)) )
					{
						// 26092008 corrected multiplication factor, prevly: e->obsProb
						// 061008 changed calculation for obsProb
						sum_o += e->obsProb * solver->beliefCacheSet[e->nextState->cacheIndex.sval]->getRow(e->nextState->cacheIndex.row)->LB;
						//sum_o += e->obsProb * Qax->obsStateProb * bounds->boundsSet[e->nextState->cacheIndex.sval]->beliefCache->getRow(e->nextState->cacheIndex.row)->LB;
					}
				}
			}
		}
// the above takes the place of this
/*		FOR (x, currentNode.Q[action].getNumStateOutcomes()) {
			problem->getObsProbVector(opv, currentNode.s, action, x);
			// problem->getOutcomeProbVector(opv, currentNode.s, action);
			FOR(o, opv.size())
			{
			    if ( ((o != (unsigned int)observation) || (x != xstate)) && opv(o) > OBS_IS_ZERO_EPS )//calculate all except the xstate and observation we are considering
				{
				  sum_o += opv(o) * bounds->boundsSet[currentNode.getNextState (action, o, x).cacheIndex.sval]->beliefCache->getRow(currentNode.getNextState (action, o, x).cacheIndex.row)->LB;
				}
			}
		}
*/
		// now add the predicted LB of chosen node, multiplied by P(o,x'|x,b,a)
		// for ref: e->obsProb * Qax->obsStateProb  
		// 061008 changed calculation for obsProb
		double oxp = Qa.stateOutcomes[xstate]->outcomes[observation]->obsProb;
		//double oxp = Qa.stateOutcomes[xstate]->outcomes[observation]->obsProb * Qa.stateOutcomes[xstate]->obsStateProb;
		value = sum_o + (oxp * bm->getBinValue( currentNode.getNextState (action, observation, xstate).cacheIndex ));
		// the above takes the place of this
		/* problem->getObsProbVector(opv, currentNode.s, action, xstate);
		value = sum_o + (opv(observation) * bm->getBinValue( currentNode.getNextState (action, observation, xstate).cacheIndex )); */

		value *= problem->getDiscount();
		value += currentNode.Q[action].immediateReward;
		
		double curTargetLb;
		if (isRoot)
		{
			curTargetLb = maxValue;
		}	
		else
		{
			curTargetLb = max(maxValue, nextNodeTargetLbArr[currentRoot]);
		}

		nextNodeTargetLbArr[currentRoot] = ((curTargetLb - currentNode.Q[action].immediateReward) / problem->getDiscount() - sum_o) / oxp;
		//nextNodeTargetLb = ((curTargetLb - currentNode.Q[action].immediateReward) / problem->getDiscount() - sum_o) / opv(observation);

		return (value > curTargetLb);
	}

// doesnt seem to be used? - SYL 23072008
	double SampleBP::calculateLowerBoundValue(int action, BeliefTreeNode & currentNode)
	{
		double value = 0;
		outcome_prob_vector opv;

		FOR (x, currentNode.Q[action].getNumStateOutcomes()) {

			problem->getObsProbVector(opv, *currentNode.s, action, x);
			FOR(o, opv.size())
			{
				if ( opv(o) > OBS_IS_ZERO_EPS )
				{	
					value += opv(o) *  solver->beliefCacheSet[currentNode.getNextState (action, o, x).cacheIndex.sval]->getRow(currentNode.getNextState (action, o, x).cacheIndex.row)->LB;
				}
			}
		}
		value *= problem->getDiscount();
		value += currentNode.Q[action].immediateReward;
		return value;
	}

	int SampleBP::chooseAction(BeliefTreeNode & currentNode)
	{
		int * actions = (int *)malloc(sizeof(int)*problem->getNumActions());
		int max_action = solver->upperBoundSet->set[currentNode.cacheIndex.sval]->dataTable->get(currentNode.cacheIndex.row).UB_ACTION;

		int index = 0;
		int action = max_action;

		FOR (a, problem->getNumActions()) {
			if ( currentNode.Q[a].ubVal >= (currentNode.Q[max_action].ubVal - 0.5*1e-9) ) {
				actions[index] = a;
				index++;
			}
		}
		action = actions[ rand() % index ];
		free(actions);
		return action;
	}
};//end namespace momdp
