#include <cerrno>
#include <cstring>
#include <iomanip>

#include "SARSOP.h"
#include "SARSOPPrune.h"
#include "MOMDP.h"
#include "BeliefValuePairPoolSet.h"
#include "AlphaPlanePoolSet.h"
#include "BeliefTreeNode.h"
#include "CPTimer.h"
#include "BlindLBInitializer.h"
#include "FastInfUBInitializer.h"
#include "BackupBeliefValuePairMOMDP.h"
#include "BackupAlphaPlaneMOMDP.h"


void printSampleBelief(list<cacherow_stval>& beliefNStates)
{
	cout << "SampledBelief" <<  endl;
	for(list<cacherow_stval>::iterator iter =beliefNStates.begin(); iter != beliefNStates.end() ; iter ++)
	{
		cout <<  "[ " <<(*iter).row << " : " << (*iter).sval << " ] ";
	}
	cout <<  endl;
}


void SARSOP::progressiveIncreasePolicyInteval(int& numPolicies)
{
	if (numPolicies == 0) 
	{
		this->solverParams->interval *= 10; 
		numPolicies++;

	} 
	else 
	{
		if (numPolicies == 5)
		{
			this->solverParams->interval *= 5;
		}
		else if (numPolicies == 10)
		{
			this->solverParams->interval *= 2;
		}
		else if (numPolicies == 15)
		{
			this->solverParams->interval *= 4;
		}

		numPolicies++;
	}
}

SARSOP::SARSOP(SharedPointer<MOMDP> problem, SolverParams * solverParams)
{
	this->problem = problem;
	this->solverParams = solverParams;
	beliefForest = new BeliefForest(); 
	sampleEngine = new SampleBP();
	((SampleBP*)sampleEngine)->setup(problem, this);
	beliefForest->setup(problem, this->sampleEngine, &this->beliefCacheSet);
	numBackups = 0;
}

SARSOP::~SARSOP(void)
{
}


void SARSOP::solve(SharedPointer<MOMDP> problem)
{
	try
	{

		bool skipSample = false;	// ADDED_24042009 flag for when all roots have ended their last trial and precision gap  <= 0

		//struct tms now;
		int policyIndex, checkIndex;//index for policy output file, and index for checking whether to output policy file
		bool stop;
		std::vector<cacherow_stval> currentBeliefIndexArr; //int currentBeliefIndex;
		// modified for parallel trials
		//cacherow_stval currentBeliefIndex; //int currentBeliefIndex;
		currentBeliefIndexArr.resize(problem->initialBeliefX->size());

		FOR(r,currentBeliefIndexArr.size())
		{
			currentBeliefIndexArr[r].sval = -1;
			currentBeliefIndexArr[r].row = -1;
		}

		list<cacherow_stval> sampledBeliefs;  //modified for factored, prevly: list<int> sampledBeliefs;
		cacherow_stval lastRootBeliefIndex; //24092008 added to keep track of root chosen for each new trial
		lastRootBeliefIndex.row = -1;
		lastRootBeliefIndex.sval = -1;

		int numPolicies = 0; //SYLADDED 07082008 temporary

		//start timing
		//times(&start);
		runtimeTimer.start();
		cout << "\nSARSOP initializing ..." << endl;

		initialize(problem);

		if(problem->XStates->size() != 1 && problem->hasPOMDPMatrices())
		{
			// only POMDPX parser can generates 2 sets of matrices, therefore, only release the second set if it is using POMDPX parser and the second set is generated
			problem->deletePOMDPMatrices();
		}
		if(problem->XStates->size() != 1 && problem->hasPOMDPMatrices())
		{
			// only POMDPX parser can generates 2 sets of matrices, therefore, only release the second set if it is using POMDPX parser and the second set is generated
			problem->deletePOMDPMatrices();
		}
		if(problem->XStates->size() != 1 && problem->hasPOMDPMatrices())
		{
			// only POMDPX parser can generates 2 sets of matrices, therefore, only release the second set if it is using POMDPX parser and the second set is generated
			problem->deletePOMDPMatrices();
		}
		GlobalResource::getInstance()->getInstance()->solving = true;

		//cout << "finished calling initialize() in SARSOP::solve()" << endl;

		//initialize parameters 
		stop = false;

		//ADD SYLTAG - need to expand global root and all the roots for sampling
		BeliefForest& globalroot = *(sampleEngine->getGlobalNode());
		beliefForest->globalRootPrepare();//do preparation work for global root

		// cycle through all the roots and do preparation work
		FOR(r, globalroot.sampleRootEdges.size()) {
			//		FOR(r, sampleEngine->globalRoot->sampleRootEdges.size()) {
			if (NULL != globalroot.sampleRootEdges[r]) 
			{
				BeliefTreeNode& thisRoot = *(globalroot.sampleRootEdges[r]->sampleRoot);
				sampleEngine->samplePrepare(thisRoot.cacheIndex);//do preparation work for this root
			}
		}

		// TODO:: sampleEngine->dumpData = dumpData;//dump data
		// TODO:: sampleEngine->dumpPolicyTrace = dumpPolicyTrace;//dump datadone
		policyIndex = 0;
		checkIndex = 0;

		lapTimer.start();
		elapsed = runtimeTimer.elapsed();
		printf("  initialization time : %.2fs\n", elapsed);


		DEBUG_LOG(logFilePrint(policyIndex-1););

		// paused timer for writing policy
		double currentElapsed = lapTimer.elapsed();
		lapTimer.pause();
		runtimeTimer.pause();

		//write out INITIAL policy

		DEBUG_LOG(writeIntermediatePolicyTraceToFile(0, 0.0, this->solverParams->outPolicyFileName, this->solverParams->problemName ); );
		DEBUG_LOG(cout << "Initial policy written" << endl;);
		
		printHeader();	

		lapTimer.resume();
		runtimeTimer.resume();

		policyIndex++;
		elapsed += currentElapsed;

		//times(&last);//renew 'last' time flag
		lapTimer.restart();

		alwaysPrint();

		DEBUG_LOG( logFilePrint(policyIndex-1); );


		//create while loop where:
		int lastTrial = ((SampleBP*)sampleEngine)->numTrials;

		// no root assigned as active at the beginning
		int activeRoot = -1;

		while(!stop)
		{
			int numTrials = ((SampleBP*)sampleEngine)->numTrials;
			if( this->solverParams->targetTrials > 0 && numTrials >  this->solverParams->targetTrials )
			{
				//    target number of trials reached
				break;
			}

			//0. IF this is the start of a new trial, 
			// backup the list of nodes in sampledBeliefs, then 
			// decide on which root to sample from
			//	(choose the root which has the largest weighted excess uncertainty)
			//   ELSE, do a regular backup of just one node

			if (activeRoot == -1) 
			{
				FOR (r, globalroot.getGlobalRootNumSampleroots()) 
				{
					SampleRootEdge* eR = globalroot.sampleRootEdges[r];

					if (NULL != eR) 
					{
						BeliefTreeNode & sn = *eR->sampleRoot;
						sampledBeliefs.clear();
						sampledBeliefs.push_back(sn.cacheIndex);

						DEBUG_TRACE( printSampleBelief(sampledBeliefs); );

						currentBeliefIndexArr[r] =  backup(sampledBeliefs);
					}
				}
				sampledBeliefs.clear();

			} 
			else  
			{

				if (((SampleBP *)sampleEngine)->newTrialFlagArr[activeRoot] == 1) 
				{
					// backup the list of nodes in sampledBeliefs
					DEBUG_TRACE( printSampleBelief(sampledBeliefs); );
					currentBeliefIndexArr[activeRoot] = backup(sampledBeliefs);
					lastRootBeliefIndex = currentBeliefIndexArr[activeRoot];
					sampledBeliefs.clear();
					// backup at all root nodes except for the root node that we had just backedup
					FOR (r, globalroot.getGlobalRootNumSampleroots()) 
					{
						SampleRootEdge* eR = globalroot.sampleRootEdges[r];

						if (NULL != eR) {
							BeliefTreeNode & sn = *eR->sampleRoot;
							// check if we had just done backup at this root,
							if( !((sn.cacheIndex.row == lastRootBeliefIndex.row)&&(sn.cacheIndex.sval == lastRootBeliefIndex.sval)) ) {

								// ADDED_24042009 - dont do LB backup if precision gap <= 0
								// check if the precision gap for this root is already zero
								double lbVal = beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->LB;
								double ubVal = beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->UB;

								if (!((ubVal - lbVal) <= 0)) 
								{
									// else, do backup at this root
									sampledBeliefs.clear();
									sampledBeliefs.push_back(sn.cacheIndex);

									DEBUG_TRACE( cout << "LB backup only " << endl; );
									DEBUG_TRACE( printSampleBelief(sampledBeliefs); );

									backupLBonly(sampledBeliefs);
									//ofsol1710d: backup(sampledBeliefs);
								} 
							}
						}
					}
					sampledBeliefs.clear();

					((SampleBP *)sampleEngine)->newTrialFlagArr[activeRoot]  = 0;

				} 
				else 
				{
					DEBUG_TRACE( printSampleBelief(sampledBeliefs); );
					currentBeliefIndexArr[activeRoot] = backup(sampledBeliefs);

				}

			}	

			// go to next valid activeRoot here
			if (activeRoot == -1) // set to the first valid root
			{
				// cycle through all roots till we find a valid one
				FOR (r, globalroot.getGlobalRootNumSampleroots()) 
				{
					SampleRootEdge* eR = globalroot.sampleRootEdges[r];
					if (NULL != eR) 
					{
						activeRoot = r;
						break;
					}
				} 
			} 
			else			// set to the next valid root
			{
				int currActiveRoot = activeRoot; 	// ADDED_24042009  
				bool passedcurrActiveRoot = false;	// ADDED_24042009 
				while(true){

					// ADDED_24042009
					if ((activeRoot == currActiveRoot) && passedcurrActiveRoot) // i.e. this is the second time that activeRoot == currActiveRoot, the while loop has cycled through all roots and not found one that passes the tests below
					{	skipSample = true;		// flag to indicate dont call sample()
					break;
					} 

					if (activeRoot == currActiveRoot) passedcurrActiveRoot = true;

					if (activeRoot == (globalroot.getGlobalRootNumSampleroots()-1)) 
						activeRoot = 0;
					else activeRoot++;

					if (globalroot.sampleRootEdges[activeRoot] != NULL){

						// ADDED_24042009 - dont go to this root if this root is about to start a new trial
						// and the precision gap for the root is already zero
						cacherow_stval currCacheIndex = globalroot.sampleRootEdges[activeRoot]->sampleRoot->cacheIndex;
						double lbVal = beliefCacheSet[currCacheIndex.sval]->getRow(currCacheIndex.row)->LB;
						double ubVal = beliefCacheSet[currCacheIndex.sval]->getRow(currCacheIndex.row)->UB;
						if (!((((SampleBP *)sampleEngine)->trialTargetPrecisionArr[activeRoot] == -1)&&((ubVal - lbVal) <= 0) ))
						{
							break;
						}
					}
				}
			}

			//2. sample
			//  samples the next belief to do backup
			//  a. if haven't reached target depth, search further
			//  b. if target depth has been reached, go back to root
			if (!skipSample) 
			{		
				// ADDED_24042009
				sampledBeliefs = sampleEngine->sample(currentBeliefIndexArr[activeRoot], activeRoot);
			}
			//3. prune
			//	decide whether needs pruning at this moment, if so,
			//  prune off the unnecessary nodes

			//DEBUG_TRACE (beliefForest->print(););

			pruneEngine->prune(); 

			//4. write out policy file if interval time reached
			// check time every CHECK_INTERVAL backups
			if(this->solverParams->interval > 0 || this->solverParams->timeoutSeconds > 0)
			{
				//only do this if required
				if((numBackups/CHECK_INTERVAL) >= checkIndex)
				{//do check every CHECK_INTERVAL(50) backups
					//times(&now);
					checkIndex++;//for next check

					//check and write out policy file periodically
					if (this->solverParams->interval > 0)
					{
						double currentElapsed = lapTimer.elapsed();
						if(currentElapsed > this->solverParams->interval)
						{
							//write out policy and reset parameters

							// paused timer for writing policy
							lapTimer.pause();
							runtimeTimer.pause();

							writeIntermediatePolicyTraceToFile(numTrials, runtimeTimer.elapsed(), this->solverParams->outPolicyFileName, this->solverParams->problemName );

							lapTimer.resume();
							runtimeTimer.resume();

							policyIndex++;
							elapsed += currentElapsed;
							cout << "Intermediate policy written(interval: "<< this->solverParams->interval <<")" << endl;

							// reset laptime so that next interval can start
							lapTimer.restart();


							DEBUG_LOG(logFilePrint(policyIndex-1););
							DEBUG_LOG( progressiveIncreasePolicyInteval(numPolicies); );


						}
					}//end write out policy periodically

					else if(this->solverParams->timeoutSeconds >0)
					{
						double currentElapsed = runtimeTimer.elapsed();
						elapsed = currentElapsed;
					}
				}//end check periodically for policy write out and elapsed time update 
			}

			//5. do printing for current precision
			print();

			//6. decide whether to stop here
			stop = stopNow();


		}

	}

	catch(bad_alloc &e)
	{
		// likely bad_alloc exception
		// should we remove the last alpha vector?
		cout << "Memory limit reached, trying to write out policy" << endl;

	}

	//prune for the last time
	FOR (stateidx, lowerBoundSet->set.size())
	{
		lowerBoundSet->set[stateidx]->pruneEngine->prunePlanes();
	}

	printHeader();
	alwaysPrint();
	printDivider();
	DEBUG_LOG(logFilePrint(-1););

	//now output policy to the outfile
	cout << endl << "Writing out policy ..." << endl;
	cout << "  output file : " << this->solverParams->outPolicyFileName << endl;
	writePolicy(this->solverParams->outPolicyFileName, this->solverParams->problemName);
}

//Function: print
//Functionality:
//	print the necessary info for help  understanding current situation inside
//	solver

void SARSOP::print()
{
	if(numBackups/CHECK_INTERVAL>printIndex)
	{
		printIndex++;
		//print time now
		alwaysPrint();
	}
}

//Function: print
//Functionality:
//    print the necessary info for help  understanding current situation inside
//    solver
void SARSOP::alwaysPrint()
{
	//struct tms now;
	//float utime, stime;
	//long int clk_tck = sysconf(_SC_CLK_TCK);

	//print time now
	//times(&now);
	double currentTime =0;
	if(this->solverParams->interval >0)
	{
		currentTime = elapsed + lapTimer.elapsed();
	}
	else
	{
		currentTime = runtimeTimer.elapsed();
	}
	//printf("%.2fs ", currentTime);
	cout.precision(6);
	cout <<" ";cout.width(8);cout << left << currentTime;

	//print current trial number, num of backups
	int numTrials = ((SampleBP*)sampleEngine)->numTrials;
	//printf("#Trial %d ",numTrials);
	cout.width(7);cout << left  <<numTrials << " "; 
	//printf("#Backup %d ", numBackups); 
	cout.width(8);cout << left << numBackups << " ";
	//print #alpha vectors
	//print precision

	//ADD SYLTAG
	//assume we can estimate lb and ub at the global root
	//by cycling through all the roots to find their bounds
	double lb = 0, ub = 0, width = 0;

	BeliefForest& globalRoot  = *(sampleEngine->getGlobalNode());
	FOR (r, globalRoot.getGlobalRootNumSampleroots()) 
	{
		SampleRootEdge* eR = globalRoot.sampleRootEdges[r];
		if (NULL != eR) 
		{
			BeliefTreeNode & sn = *eR->sampleRoot;
			double lbVal =	beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->LB;
			double ubVal =	beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->UB;
			lb += eR->sampleRootProb * lbVal;
			ub += eR->sampleRootProb * ubVal;
			width += eR->sampleRootProb * (ubVal - lbVal);
		}
	}

	//REMOVE SYLTAG
	//cacherow_stval rootIndex = sampleEngine->getRootNode()->cacheIndex;
	//double lb = bounds->boundsSet[rootIndex.sval]->beliefCache->getRow(rootIndex.row)->LB;
	//double ub = bounds->boundsSet[rootIndex.sval]->beliefCache->getRow(rootIndex.row)->UB;

	//printf("[%f,%f],", lb, ub);
	cout.width(10); cout << left << lb<< " ";
	cout.width(10); cout << left << ub<< " ";
	
	//print precision
	double precision = width; // ub - lb;   //MOD SYLTAG
	//printf("%f, ", precision);
	cout.width(11);	
	cout << left << precision << " ";
	int numAlphas = 0;
	FOR (setIdx, beliefCacheSet.size()) 
	{
		numAlphas += (int)lowerBoundSet->set[setIdx]->planes.size();
	}

	//printf("#Alphas %d ", numAlphas);			//SYLTEMP FOR EXPTS
	cout.width(9);cout << left << numAlphas;

	//print #belief nodes
	//printf("#Beliefs %d", sampleEngine->numStatesExpanded);
	cout.width(9);cout << left << sampleEngine->numStatesExpanded;

	//printf("#alphas %d", (int)bounds->alphaPlanePool->planes.size());
	printf("\n"); 

}

//SYL ADDED FOR EXPTS
//Function: print
//Functionality:
//    print the necessary info for help  understanding current situation inside
//    solver

void SARSOP::logFilePrint(int index)
{
	//struct tms now;
	//float utime, stime;
	//long int clk_tck = sysconf(_SC_CLK_TCK);

	//print time now
	//times(&now);

	FILE *fp = fopen("solve.log", "a");
	if(fp==NULL){
		cerr << "can't open logfile\n";
		exit(1);
	}


	fprintf(fp,"%d ",index);

	//print current trial number, num of backups
	int numTrials = ((SampleBP*)sampleEngine)->numTrials;
	//int numBackups = numBackups;
	fprintf(fp,"%d ",numTrials); 			//SYLTEMP FOR EXPTS
	//printf("#Trial %d, #Backup %d ",numTrials, numBackups); 

	//print #alpha vectors
	int numAlphas = 0;
	FOR (setIdx, beliefCacheSet.size()) 
	{
		//cout << " p : " << setIdx << " : " << 	 (int)bounds->boundsSet[setIdx]->alphaPlanePool->planes.size();
		numAlphas += (int)lowerBoundSet->set[setIdx]->planes.size();
	}

	fprintf(fp, "%d ", numAlphas);			//SYLTEMP FOR EXPTS

	double currentTime =0;
	if(this->solverParams->interval >0)
	{
		//utime = ((float)(now.tms_utime-last.tms_utime))/clk_tck;
		//stime = ((float)(now.tms_stime-last.tms_stime))/clk_tck;
		//currentTime = elapsed+utime+stime;
		currentTime = elapsed + lapTimer.elapsed();
		fprintf(fp, "%.2f ", currentTime);			//SYLTEMP FOR EXPTS
		//printf("<%.2fs> ", currentTime);
	}
	else{
		//utime = ((float)(now.tms_utime-start.tms_utime))/clk_tck;
		//stime = ((float)(now.tms_stime-start.tms_stime))/clk_tck;
		//currentTime = utime+stime;
		currentTime = runtimeTimer.elapsed();
		fprintf(fp, "%.2f ", currentTime);		//SYLTEMP FOR EXPTS
		//printf("<%.2fs> ", currentTime);	
	}

	fprintf(fp,"\n"); 

	fclose(fp);
}

//ADD SYLTAG
bool SARSOP::stopNow(){
	bool stop = false;

	double width = 0;
	BeliefForest& globalRoot  = *(sampleEngine->getGlobalNode());

	//find the weighted excess uncertainty at the global root
	//cycle through all the roots to find their bounds
	FOR (r, globalRoot.getGlobalRootNumSampleroots()) 
	{
		SampleRootEdge* eR = globalRoot.sampleRootEdges[r];
		if (NULL != eR) {
			BeliefTreeNode & sn = *eR->sampleRoot;
			double lbVal =	beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->LB;
			double ubVal =	beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->UB;
			width += eR->sampleRootProb * (ubVal - lbVal);
		}
	}

	if(GlobalResource::getInstance()->userTerminatedG)
	{
		stop = true;
	}

	if ((width) < this->solverParams->targetPrecision)
	{      
		alwaysPrint();
		printDivider();
		printf("\nSARSOP finishing ...\n");
		printf("  target precision reached\n");
		printf("  target precision  : %f\n", this->solverParams->targetPrecision);
		printf("  precision reached : %f \n", width);

		stop = true;
	}
	if (this->solverParams->timeoutSeconds > 0)
	{
		if (elapsed > this->solverParams->timeoutSeconds )
		{
			printDivider();
			printf("\nSARSOP finishing ...\n");
			printf("  Preset timeout reached\n");
			printf("  Timeout     : %fs\n",  this->solverParams->timeoutSeconds );
			printf("  Actual Time : %fs\n", elapsed);
			stop = true;
		}
	}
	return stop;
}


//REMOVE SYLTAG
/*	bool SARSOP::stopNow(){
bool stop = false;
cacherow_stval rootIndex = sampleEngine->getRootNode()->cacheIndex;
//int rootIndex = sampleEngine->getRootNode()->cacheIndex;

//decide whether to stop or not depend on current root precision
double lb = bounds->boundsSet[rootIndex.sval]->beliefCache->getRow(rootIndex.row)->LB;
double ub = bounds->boundsSet[rootIndex.sval]->beliefCache->getRow(rootIndex.row)->UB;
#if USE_DEBUG_PRINT
printf("targetPrecision is %f, precision is %f\n", targetPrecision, ub-lb);
#endif
if(GlobalResource::getInstance()->userTerminatedG)
{
stop = true;
}
if ((ub-lb)<targetPrecision){      
alwaysPrint();
printf("Target precision reached: %f (%f)\n\n", ub-lb, targetPrecision);
stop = true;
}
if (timeout > 0){
if (elapsed > timeout){
printf("Preset timeout reached %f (%fs)\n\n", elapsed, timeout);
stop = true;
}
}
return stop;
}
*/

void SARSOP::writeIntermediatePolicyTraceToFile(int trial, double time, const string& outFileName, string problemName)
{
	stringstream newFileNameStream;
	string outputBasename = GlobalResource::parseBaseNameWithPath(outFileName);
	newFileNameStream << outputBasename << "_" << trial << "_" << time << ".policy";
	string newFileName = newFileNameStream.str();
	cout << "Writing policy file: " << newFileName << endl;
	writePolicy(newFileName, problemName);
}


BeliefTreeNode& SARSOP::getMaxExcessUncRoot(BeliefForest& globalroot) 
{

	double maxExcessUnc = -99e+20;
	int maxExcessUncRoot = -1;
	double width;
	double lbVal, ubVal;

	FOR (r, globalroot.getGlobalRootNumSampleroots()) {
		SampleRootEdge* eR = globalroot.sampleRootEdges[r];
		if (NULL != eR) {
			BeliefTreeNode & sn = *eR->sampleRoot;
			lbVal =	beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->LB;
			ubVal =	beliefCacheSet[sn.cacheIndex.sval]->getRow(sn.cacheIndex.row)->UB;
			width =	eR->sampleRootProb * (ubVal - lbVal);

			if (width > maxExcessUnc)
			{
				maxExcessUnc = width;
				maxExcessUncRoot = r;
			}
		}

	}

	return *(globalroot.sampleRootEdges[maxExcessUncRoot]->sampleRoot);

}

void SARSOP::backup(BeliefTreeNode* node)
{
	upperBoundSet->backup(node);
	lowerBoundSet->backup(node);
}

void SARSOP::initialize(SharedPointer<MOMDP> problem)
{
	printIndex = 0; // reset printing counter

	int xStateNum = problem->XStates->size();
	beliefCacheSet.resize(xStateNum);
	lbDataTableSet.resize(xStateNum);
	ubDataTableSet.resize(xStateNum);

	for(States::iterator iter = problem->XStates->begin(); iter != problem->XStates->end(); iter ++ )
	{
		beliefCacheSet[iter.index()] = new BeliefCache();
		lbDataTableSet[iter.index()] = new IndexedTuple<AlphaPlanePoolDataTuple>();
		ubDataTableSet[iter.index()] = new IndexedTuple<BeliefValuePairPoolDataTuple>();
	}

	initializeUpperBound(problem);

	upperBoundSet->setBeliefCache(beliefCacheSet);
	upperBoundSet->setDataTable(ubDataTableSet);

	initializeLowerBound(problem);
	lowerBoundSet->setBeliefCache(beliefCacheSet);
	lowerBoundSet->setDataTable(lbDataTableSet);

	initializeBounds(this->solverParams->targetPrecision);
	initSampleEngine(problem);

	pruneEngine = new SARSOPPrune(this);

}
void SARSOP::initSampleEngine(SharedPointer<MOMDP> problem)
{
	sampleEngine->appendOnGetNodeHandler(&SARSOP::onGetNode);
	binManagerSet = new BinManagerSet(upperBoundSet);
	((SampleBP*)sampleEngine)->setBinManager(binManagerSet);
	((SampleBP*)sampleEngine)->setRandomization(solverParams->randomizationBP);

}
void SARSOP::initializeUpperBound(SharedPointer<MOMDP> problem)
{
	upperBoundSet = new BeliefValuePairPoolSet(upperBoundBackup);
	upperBoundSet->setProblem(problem);
	upperBoundSet->setSolver(this);
	upperBoundSet->initialize();
	upperBoundSet->appendOnBackupHandler(&SARSOP::onUpperBoundBackup);
	((BackupBeliefValuePairMOMDP*)upperBoundBackup)->boundSet = upperBoundSet;
}
void SARSOP::initializeLowerBound(SharedPointer<MOMDP> problem)
{
	lowerBoundSet = new AlphaPlanePoolSet(lowerBoundBackup);
	lowerBoundSet->setProblem(problem);
	lowerBoundSet->setSolver(this);
	lowerBoundSet->initialize();
	lowerBoundSet->appendOnBackupHandler(&SARSOP::onLowerBoundBackup);
	lowerBoundSet->appendOnBackupHandler(&SARSOPPrune::onLowerBoundBackup);
	((BackupAlphaPlaneMOMDP* )lowerBoundBackup)->boundSet = lowerBoundSet;
}

void SARSOP::initializeBounds(double _targetPrecision)
{
	double targetPrecision = _targetPrecision * CB_INITIALIZATION_PRECISION_FACTOR;

	CPTimer heurTimer;
	heurTimer.start(); 	// for timing heuristics
	BlindLBInitializer blb(problem, lowerBoundSet);
	blb.initialize(targetPrecision);
	elapsed = heurTimer.elapsed();

	DEBUG_LOG( cout << fixed << setprecision(2) << elapsed << "s blb.initialize(targetPrecision) done" << endl; );

	heurTimer.restart();
	FastInfUBInitializer fib(problem, upperBoundSet); 
	fib.initialize(targetPrecision);
	elapsed = heurTimer.elapsed();
	DEBUG_LOG(cout << fixed << setprecision(2) << elapsed << "s fib.initialize(targetPrecision) done" << endl;);

	FOR (state_idx, problem->XStates->size()) 
	{
		upperBoundSet->set[state_idx]->cornerPointsVersion++;	// advance the version by one so that next time get value will calculate rather than skip
	}

	//cout << "finished setting cornerPointsVersion" << endl;

	numBackups = 0;
}//end method initialize

cacherow_stval SARSOP::backup(list<cacherow_stval> beliefNStates)
{
	//decide the order of backups 
	cacherow_stval rowNState, nextRowNState;
	nextRowNState.row = -1;

	//for each belief given, we perform backup for it
	LISTFOREACH(cacherow_stval, beliefNState,  beliefNStates) 
	{
		//get belief
		rowNState = *beliefNState;
		nextRowNState = backup(rowNState);
	}//end FOR_EACH

	// prevly:
	//for each belief given, we perform backup for it
	/* LISTFOREACH(int, belief,  beliefs) {
	//get belief
	row = *belief;
	nextRow = backup(row);
	}//end FOR_EACH */



	if(nextRowNState.row== -1)
	{
		printf("Error: backup list empty\n");
		cout << "In SARSOP::backup( )" << endl;
	}
	return nextRowNState;
}//end method: backup


cacherow_stval SARSOP::backupLBonly(list<cacherow_stval> beliefNStates){
	//decide the order of backups 
	cacherow_stval rowNState, nextRowNState;
	nextRowNState.row = -1;

	//for each belief given, we perform backup for it
	LISTFOREACH(cacherow_stval, beliefNState,  beliefNStates) {
		//get belief
		rowNState = *beliefNState;
		nextRowNState = backupLBonly(rowNState);
	}//end FOR_EACH


	if(nextRowNState.row== -1){
		printf("Error: backup list empty\n");
		cout << "In SARSOP::backupLBonly( )" << endl;
	}
	return nextRowNState;
}//end method: backup

//Function: backup
//Functionality:
//	do backup at a single belief
//Parameters:
//	row: the row index of the to-be-backuped belief in BeliefCache
//Returns:
//	row index of the belief as the starting point for sampling
cacherow_stval SARSOP::backup(cacherow_stval beliefNState)
{

	unsigned int stateidx = beliefNState.sval;
	int row = beliefNState.row;

	//cout << "in SARSOP::backup(), beliefNState.sval : " << beliefNState.sval << " beliefNState.row : " << beliefNState.row << endl;

	//do belief propogation if the belief is a fringe node in tree
	BeliefTreeNode* cn = beliefCacheSet[stateidx]->getRow(row)->REACHABLE;
	//should we use BeliefTreeNode or should we use row index?
	// TEMP, should move all the time stamp code to Global Resource

	//SYL220210 the very first backup should have timestamp of 1, so we increment the timestamp first 
	numBackups++;
	
	GlobalResource::getInstance()->setTimeStamp(numBackups);
	lowerBoundSet->backup(cn);
	upperBoundSet->backup(cn);
	//numBackups++;
	GlobalResource::getInstance()->setTimeStamp(numBackups);
	return beliefNState;
}

cacherow_stval SARSOP::backupLBonly(cacherow_stval beliefNState){

	unsigned int stateidx = beliefNState.sval;
	int row = beliefNState.row;

	//cout << "in SARSOP::backup(), beliefNState.sval : " << beliefNState.sval << " beliefNState.row : " << beliefNState.row << endl;

	//do belief propogation if the belief is a fringe node in tree
	BeliefTreeNode* cn = beliefCacheSet[stateidx]->getRow(row)->REACHABLE;
	//should we use BeliefTreeNode or should we use row index?
	// TEMP, should move all the time stamp code to Global Resource
	GlobalResource::getInstance()->setTimeStamp(numBackups);
	lowerBoundSet->backup(cn);
	//bounds->backupUpperBoundBVpair->backup(*cn);
	numBackups++;
	GlobalResource::getInstance()->setTimeStamp(numBackups);
	return beliefNState;
}

void SARSOP::writePolicy(string fileName, string problemName)
{
	writeToFile(fileName, problemName);
}

void SARSOP::writeToFile(const std::string& outFileName, string problemName) 
{
	lowerBoundSet->writeToFile(outFileName, problemName);

}

void SARSOP::printHeader(){
    cout << endl;
    printDivider();
    cout << " Time   |#Trial |#Backup |LBound    |UBound    |Precision  |#Alphas |#Beliefs  " << endl;
    printDivider();
}

void SARSOP::printDivider(){
    cout << "-------------------------------------------------------------------------------" << endl;
}
