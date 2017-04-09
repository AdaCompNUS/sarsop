#include "SimulationRewardCollector.h"

SimulationRewardCollector::SimulationRewardCollector(void)
{
}

SimulationRewardCollector::~SimulationRewardCollector(void)
{
}


void SimulationRewardCollector::setup(SolverParams& p)
{
	this->p = p;
	expRewardRecord.resize(p.simNum);
	totalVar = 0;
	globalRew = 0;
	globalExpRew = 0;
	confInterval = 0;
	legendPrint = true; // print legends for acronyms used
	
}
void SimulationRewardCollector::addEntry(int currSim, double reward, double expReward)
{
	globalRew += reward / p.simNum;
	globalExpRew += expReward / p.simNum;
	expRewardRecord[currSim] = expReward;
}

void SimulationRewardCollector::printReward(int currSim)
{
	int simDisplayInc = int(p.simNum / 10);
	if(simDisplayInc == 0)
	{
		simDisplayInc = 1;
	}
	if( legendPrint)
	{
	        cout << endl; 
		cout << "-----------------------------------" << endl;
		cout << " #Simulations  | Exp Total Reward  "<< endl;
		cout << "-----------------------------------"<< endl;
		legendPrint = false;
	}

	if (((currSim + 1) % (simDisplayInc)) == 0) 
	{
		//cout << p.policyFile << endl;
		//cout << "#Simulations = " << currSim + 1;
		cout << " "; cout.width(15);cout <<left << currSim+1;
		cout << " " << left <<  (globalExpRew * p.simNum)/ (currSim + 1) << endl;
		// " Global Reward = " << (globalRew * p.simNum) / (currSim + 1) <<
	}
}

void SimulationRewardCollector::calculateConfidenceInterval()
{
	double currVar = 0;
	for (int currSim = 0; currSim < p.simNum; currSim++) 
	{
		currVar = pow((expRewardRecord[currSim] - globalExpRew), 2);
		totalVar += currVar / p.simNum;
	}
	totalVar = sqrt(totalVar); // sample's stddev
	confInterval = 1.96 * totalVar / sqrt((double)p.simNum); // stddev_mean = stddev_sample/sqrt(simNum)
}

void SimulationRewardCollector::printFinalReward()
{
	calculateConfidenceInterval();
	cout << "-----------------------------------"<< endl << endl;

	cout << "Finishing ..." << endl << endl;
// 	cout << "Policy file: " << p.policyFile << endl;
	//cout << globalExpRew << " " << globalExpRew - confInterval << " " << globalExpRew + confInterval << endl; //- TEMP FORMAT FOR EXPTS
	cout << "-------------------------------------------------------------"<< endl;
	cout << " #Simulations  | Exp Total Reward | 95% Confidence Interval "<< endl;
	cout << "-------------------------------------------------------------"<< endl;
	//cout << "#Simulation = " << p.simNum;
	// cout << " ETR = " << globalExpRew << " 95% Confidence Interval = (" << globalExpRew - confInterval << ", " << globalExpRew + confInterval << ")" << endl;
	cout << " "; cout.width(15);cout <<left << p.simNum  ;
	cout << " ";cout.width(18);cout << left << globalExpRew;
	cout << " (" <<  globalExpRew - confInterval << ", " << globalExpRew + confInterval << ")" << endl;
	cout << "-------------------------------------------------------------"<< endl;
	// << confInterval << ", 95 Conf Min = " << globalExpRew - confInterval << ", 95 Conf Max = " << globalExpRew + confInterval << endl;
	// << ", Global Exp Reward Stddev = " << totalVar	
	//" Global Reward = " << globalRew << 

}

