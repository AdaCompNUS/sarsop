/** 
* The code is released under GPL v2 
*/

#include <sstream>
#include "GlobalResource.h"
#include "CPMemUtils.h"

using namespace std;

namespace momdp
{

	GlobalResource* GlobalResource::singleInstance = NULL;

	void GlobalResource::incTimeStamp()
	{
		timeStamp ++;
	}
	int GlobalResource::getTimeStamp()
	{
		return timeStamp;
	}
	void GlobalResource::setTimeStamp(int newTimeStamp)
	{
		timeStamp = newTimeStamp;
	}

	double GlobalResource::getRunTime()
	{
		return solvingTimer.elapsed();
	}

	void GlobalResource::init()
	{
	}

	GlobalResource::GlobalResource() 
	{
		problem = NULL;
		noPolicyOutput = false;

		checkMemoryInterval = 0;
		benchmarkMode = false;
		simLookahead = false;
		randSeed = time(0);
		migsPathFile = NULL;
		migsPathFileNum = -1;

		binaryPolicy = false;
		migsUniformSamplingMileStone = false;

		timeStamp = 0;
		solving = false;
		noSarsop = false;

		currTrial = 0;

		userTerminatedG = false;
		mdpSolution = false;


		pomdpLoadTime = -1.0;
		pomdpInitializationTime = -1.0;

		// stats
		hashCollision = 0;
		hashRequest = 0;
		hashSame = 0;

		nInterStates = 0;
		nSamples = 0;
		nTimes = 0;
		th = 0.0;
		epi = 0.0;
		stateGraphR = 0.0;
		nInitMil = 0;

		trialInterval = 0;

		gesGenStateMapTime = 0.0;
		gesGenGuideMapTime = 0.0;
		gesSampleTime = 0.0;
		gesTrialTime = 0.0;
		totUpdGuideMapTime = 0.0;
		limNotImproved =0.0;

		wallClockTotalTimer.start();
		//solvingTimer;
		//solvingOneTrialTimer;
		lastIntervalSaveTime = 0.0;

		pbSolver = NULL;

		memoryUsage = 0;

		logLevel = 0;

		policyIndex = 0;
	}

	void GlobalResource::PBSolverPostInit()
	{
		pomdpInitializationTime = solvingTimer.elapsed();
		printf("time spent in initializing [%.2fs] \n", pomdpInitializationTime);
		//solvingTimer.restart();
		if(pbSolver->solverParams->interval > 0 )
		{
			saveIntermediatePolicy();
		}

	}

	string GlobalResource::parseBaseNameWithPath(string name)
	{
		int extIndex = name.find_last_of(".");
		if(extIndex ==string::npos)
		{
			return name;
		}
		return name.substr(0, extIndex);
	}
	string GlobalResource::parseBaseNameWithoutPath(string name)
	{
		//string lowerName = name;
		//transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
		//int extIndex = lowerName.find_last_of(".pomdp") - 5;

		string baseName = parseBaseNameWithPath(name);
		int slashIndex = baseName.find_last_of("/") ;
		int backSlashIndex = baseName.find_last_of("\\");
		//cout << "slashIndex  " << slashIndex << endl;
		//cout << "backSlashIndex " << backSlashIndex << endl;

		int start = 0;
		if(slashIndex != string::npos && backSlashIndex != string::npos)
		{
			start = max(slashIndex, backSlashIndex);
		}

		if(slashIndex != string::npos)
		{
			start = slashIndex;
		}

		if(backSlashIndex != string::npos )
		{
			start = backSlashIndex ;
		}
		start += 1;
		string temp = name.substr(start);
		//cout << "Basename: " << temp << endl;
		return temp;
	}


	void GlobalResource::PBSolverFinishOneTrial()
	{
		// print out stats
		//

		// How to get #alpha-vectors ? no AlphaPlanePool here.
		// Is timeStamp == #backups ?
		// rootNodeIndex in beliefCache, is it always 0 ?
		gesTrialTime = solvingTimer.elapsed() - gesTrialTime;


		// print stats
		/*
		printf("Time for backup %.2f s, #alpha %d, #backups %d, V(b0) %.5f\n", 	
		gesTrialTime, 
		pbSolver->bounds->alphaPlanePool->planes.size() , 
		getTimeStamp(),
		pbSolver->bounds->beliefCache->getRow( 0)->LB );

		*/
		currTrial ++;
		//printf("Time for backup %.2f s, #alpha %d, #backups %d, V(b0) %.5f\n", 	solvingTimer.elapsed(), pbSolver->bounds->alphaPlanePool->planes.size() , getTimeStamp(),	pbSolver->beliefCache->getRow( 0)->LB );
		//printf("Time elapsed %.2f s, #alpha %d, #backups %d, V(b0) %.5f\n",
		//	solvingTimer.elapsed(),
		//	pbSolver->bounds->alphaPlanePool->planes.size() , 
		//	getTimeStamp(),
		//	pbSolver->beliefCache->getRow( 0)->LB );
		printf("TODO HERE....\n");

		if(trialInterval > 0 )
		{
			if((currTrial % trialInterval) == 0)
			{
				saveIntermediatePolicy();
			}
		}
		//check and write out policy file periodically
		if(pbSolver->solverParams->interval > 0 )
		{
			double currentElapsed = solvingTimer.elapsed() - lastIntervalSaveTime;
			if(currentElapsed > pbSolver->solverParams->interval)
			{
				saveIntermediatePolicy();

				//pbSolver->solverParams->interval = pbSolver->solverParams->interval * 2;

				if(pbSolver->solverParams->interval > 60)
				{
					pbSolver->solverParams->interval = 60;
				}
			}
		}

	}

	void GlobalResource::saveIntermediatePolicy()
	{
		//write out policy and reset parameters
		// paused timer for writing policy
		solvingTimer.pause();

		stringstream fileName;
		fileName << baseName<< "." << policyIndex << ".policy";
		stringstream logLine;
		logLine << "IntermediatePolicy," << fileName.str() << "," << solvingTimer.elapsed() ;
		if(trialInterval > 0)
		{
			logLine << "," << currTrial;
		}
		logEntry(logLine.str());

		policyIndex++;
		if(benchmarkMode)
		{
			this->Benchmark();
			cout << "Intermediate policy evaluated(interval: " << pbSolver->solverParams->interval << ") "  << endl;
		}
		else
		{
			pbSolver->writePolicy(fileName.str(), "");
			cout << "Intermediate policy written(interval: " << pbSolver->solverParams->interval << "): " << fileName.str() << endl;
		}
		solvingTimer.resume();
		//times(&last);//renew 'last' time flag
		lastIntervalSaveTime = solvingTimer.elapsed();
	}

	GlobalResource::~GlobalResource()
	{
		shutdown();
	}



	void GlobalResource::checkMemoryUsage()
	{
		// check if memory usage is over limit
		if(solverParams.memoryLimit != 0)
		{
			// if over limit , throw bad_alloc
			//if(memoryUsage > memoryLimit)
			if(checkMemoryInterval >= MEMORY_USAGE_CHECK_INTERVAL)
			{
				checkMemoryInterval = 0;
				if(getCurrentProcessMemoryUsage() > solverParams.memoryLimit)
				{
					bad_alloc ba;
					throw ba;
					//throw new bad_alloc();
				}
			}
			else
			{
				checkMemoryInterval++;
			}

		}
	}

	void GlobalResource::Benchmark()
	{
		// TODO : benchmark
		//belief_vector startBel;
		//copy(startBel, problem->initialBelief);
		//startBel.finalize();
		//double globalRew = 0.0;
		//double globalExpRew = 0.0;

		//for(int currSim = 0;currSim < simNum; currSim++)
		//{
		//	unsigned int firstAction = 0;

		//	std::ostringstream mystrm1;

		//	BenchmarkEngine engine(*problem, *actS,tree);

		//	if(startBel.filled() == 0)
		//	{
		//		int numStates = problem->getBeliefSize();
		//		startBel.resize(numStates);
		//		for(int i = 0;i<numStates; i++){
		//			startBel.push_back(i, ((double)1)/(double(numStates)));
		//		}
		//	}

		//	double reward = 0, expReward = 0;
		//	try
		//	{
		//		reward = 0;
		//		expReward = 0;
		//		firstAction = engine.runFor(simLen, startBel, reward, expReward, false);
		//	}
		//	catch(exception &e)
		//	{
		//		cout << "Memory limit reached, continuing without allocating new data structure";
		//		cout << endl;
		//		GlobalResource::getInstance()->memoryLimit = 0; // to stop new operator of DenseVector setting off bad_alloc
		//		// should redo current round without allocating new memory
		//		reward = 0;
		//		expReward = 0;
		//		firstAction = engine.runFor(simLen, startBel, reward, expReward, true);
		//	}

		//	//SYLMOD 07082008 temporary
		//	//    cout << "Sim "<< currSim << ": reward = " <<reward <<", expected reward = " << expReward<<endl;

		//	globalRew += reward/simNum;
		//	globalExpRew += expReward/simNum;
		//	expRewardRecord[currSim] = expReward;  //SYLADDED 07082008
		//}

		////SYLADDED 07082008 section below
		//double totalVar = 0;
		//double currVar;

		//for(int currSim = 0;currSim < simNum; currSim++)
		//{
		//	currVar = pow((expRewardRecord[currSim] - globalExpRew),2);
		//	//cout << "expRewardRecord[currSim] : " << expRewardRecord[currSim] << " currVar : " << currVar << endl;
		//	totalVar += currVar/simNum;
		//}

		//totalVar = sqrt(totalVar);	// sample's stddev
		//double confInterval = 1.96 * totalVar/sqrt((double)simNum);	// stddev_mean = stddev_sample/sqrt(simNum)
		////------------

		////SYLADDED 07082008 additional outputs to screen
		//ostringstream stringStream;
		//
		////cout << globalExpRew << " " << globalExpRew - confInterval << " " << globalExpRew + confInterval << endl;		//- TEMP FORMAT FOR EXPTS
		//stringStream << "simNum = " << simNum<< " simLen = " << simLen;
		//stringStream << " Global Reward = "<< globalRew << ", Global Exp Reward = " << globalExpRew << ", Global Exp Reward Stddev = " << totalVar<< ", 95Conf Interval = " << confInterval  << ", 95Conf Min = " << globalExpRew - confInterval << ", 95Conf Max = " << globalExpRew 
		//	+ confInterval << endl;
		//this->logEntry(stringStream.str());
	}


	
}

