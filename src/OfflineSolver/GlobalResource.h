/** 
* The code is released under GPL v2 
*/
#ifndef GLOBALRESOURCE_H
#define GLOBALRESOURCE_H

#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "BeliefCache.h"
#include "PointBasedAlgorithm.h"
#include "CPTimer.h"
#include "solverUtils.h"
//#include "ActionSelector.h"
//#include "BenchmarkEngine.h"

#define MEMORY_USAGE_CHECK_INTERVAL 100

using namespace std;

namespace momdp
{
	class GlobalResource
	{
	private:
		int timeStamp;
		//	CPTimer*	runtimeTimer;

		GlobalResource();
		int checkMemoryInterval;


	public:
		SolverParams solverParams;

		char* migsPathFile;
		signed int migsPathFileNum;
		bool migsUniformSamplingMileStone;

		bool noPolicyOutput;
		bool binaryPolicy;
		int trialInterval;
		int currTrial;
		bool noSarsop;

		SharedPointer<MOMDP> problem;

		~GlobalResource();

		// Simulation, Evaluation Related 
		bool simLookahead;
		int simLen;
		int simNum;
		bool benchmarkMode;
		int randSeed;

		//ActionSelector* actS;
		std::vector<double> expRewardRecord;
		//GESTree *tree;

		bool userTerminatedG; // the Ctrl+C flag set by the OfflineEngine.cc

		bool solving;
		bool mdpSolution;


		int getTimeStamp();
		void incTimeStamp(); // increment Time Stamp
		void setTimeStamp(int newTimeStamp); //set Time Stamp
		void init(); //dummy function,  run this once in the beginning of the program to ensure the object is created
		double getRunTime(); // get the run time from the start, wall clock time

		int hashCollision;
		int hashRequest;
		int hashSame;

		int nInterStates;
		int nSamples;
		int nTimes;

		double th;
		double epi;
		double stateGraphR;
		int nInitMil;

		unsigned long memoryUsage; // in bytes

		//typedef int (*pbSolverHookType)(PointBasedAlgorithm *);

		// vector<vector < pbSolverHookType> > pbSolverHooks;
		CPTimer wallClockTotalTimer;
		CPTimer solvingTimer;
		CPTimer solvingOneTrialTimer;


		//------------------------- stats variable
		double pomdpLoadTime; // the time used to load POMDP spec files, < 0 means we are still loading it
		double pomdpInitializationTime; // the time used to initialize, < 0 means we are still loading it
		double gesGenStateMapTime;
		double gesGenGuideMapTime;
		double gesSampleTime;
		double gesTrialTime;
		double updGuideMapStartTime;
		double totUpdGuideMapTime;
		double limNotImproved;

		// for dumping at interval
		double lastIntervalSaveTime;
		PointBasedAlgorithm* pbSolver;
		int policyIndex;
		void checkMemoryUsage();

		// singleton pattern

		static GlobalResource* singleInstance;
		static GlobalResource* getInstance()
		{
			if( singleInstance == NULL)
			{
				singleInstance = new GlobalResource();
			}
			return singleInstance;
		}


		// base name for current solution
		string baseName;

		static string parseBaseNameWithPath(string name);
		static string parseBaseNameWithoutPath(string name);

		void setBaseName(string basename)
		{
			baseName = basename;
		}
		string getBaseName()
		{
			return baseName;
		}

		// log files

		int logLevel;
		ofstream logFile;
		void shutdown()
		{
			logFile.flush();
			logFile.close();
		}

		void setLogLevel(int level)
		{
			logLevel = level;
		}

		// used to log some custom info 
		void logEntry(string log)
		{
			if(!logFile.is_open())
			{
				string logFileName = baseName+".log";
				logFile.open(logFileName.c_str());
			}
			logFile << log << endl;
		}

		void logInfo(string log)
		{
			if(logLevel > 2)
			{
				logEntry(log);
			}
		}

		void logWarn(string log)
		{
			if(logLevel > 1)
			{
				logEntry(log);
			}
		}

		void logError(string log)
		{
			if(logLevel > 0)
			{
				logEntry(log);
			}
		}

		void logCritical(string log)
		{
			logEntry(log);
		}

		void registerPBSolver(PointBasedAlgorithm* pSolver)
		{
			pbSolver = pSolver;
		}

		void PBSolverPrePOMDPLoad()
		{
			wallClockTotalTimer.start();
		}

		double PBSolverPostPOMDPLoad()
		{
			pomdpLoadTime = wallClockTotalTimer.elapsed();
			return pomdpLoadTime;
		}

		void PBSolverPreInit()
		{
			solvingTimer.start();
		}
		void PBSolverPostInit();

		void PBSolverPreGESSample()
		{
			gesSampleTime = solvingTimer.elapsed();
		}
		void PBSolverPostGESSample()
		{
			gesSampleTime = solvingTimer.elapsed() - gesSampleTime;
			printf("time spent in GES Sample [%.2fs] \n", gesSampleTime);
		}

		void PBSolverPreGESGenStateMap()
		{
			gesGenStateMapTime = solvingTimer.elapsed();
		}
		void PBSolverPostGESGenStateMap()
		{
			gesGenStateMapTime = solvingTimer.elapsed() - gesGenStateMapTime;
			printf("time spent in GES GenStateMap [%.2fs] \n", gesGenStateMapTime);
		}

		void PBSolverPreGESGenGuideMap()
		{
			gesGenGuideMapTime = solvingTimer.elapsed();
		}
		void PBSolverPostGESGenGuideMap()
		{
			gesGenGuideMapTime = solvingTimer.elapsed() - gesGenGuideMapTime;
			printf("time spent in GES GenGuideMap [%.2fs] \n", gesGenGuideMapTime);
		}

		void PBSolverBeginOneTrial()
		{
			gesTrialTime = solvingTimer.elapsed();
		}

		void saveIntermediatePolicy();

		void PBSolverFinishOneTrial();

		bool PBSolverShouldStop(double ub, double lb)
		{
			bool stop = false;
			if(userTerminatedG)
			{
				stop = true;
			}

			
			if ((ub-lb) < pbSolver->solverParams->targetPrecision)
			{      
				printf("Target precision reached: %f (%f)\n\n", ub-lb, pbSolver->solverParams->targetPrecision);
				stop = true;
			}


			if(pbSolver->solverParams->timeoutSeconds > 0)
			{
				if(solvingTimer.elapsed() > pbSolver->solverParams->timeoutSeconds )
				{
					printf("Preset timeout reached %f (%fs)\n\n", solvingTimer.elapsed(), pbSolver->solverParams->timeoutSeconds);
					stop = true;
				}
			}
			return stop;
		}

		void PBSolverPreSavePolicy()
		{
			printf("Total time spent in Solving (everything included)[%.2fs] \n", solvingTimer.elapsed());
			printf("... writing policy to '%s'\n", pbSolver->solverParams->outPolicyFileName.c_str());
		}
		void PBSolverPostSavePolicy()
		{

		}
		void PBSolver()
		{
		}


		void PBPreGESUpdGuideMap()
		{
			updGuideMapStartTime = solvingTimer.elapsed();
		}

		void PBPostGESUpdGuideMap()
		{
			totUpdGuideMapTime += (solvingTimer.elapsed() - updGuideMapStartTime);
		}

		void Benchmark();
	};
};

#endif
