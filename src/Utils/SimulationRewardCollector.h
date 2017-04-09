#ifndef SimulationRewardCollector_H
#define SimulationRewardCollector_H
#include <cmath>
#include <vector>


#include "solverUtils.h"

using namespace momdp;
using namespace std;
namespace momdp 
{

class SimulationRewardCollector
{
private:
	double globalRew;
	double totalVar;

	vector<double> expRewardRecord;

	SolverParams p;

	bool legendPrint;

	void calculateConfidenceInterval();
public:
	double globalExpRew;
	double confInterval;

	SimulationRewardCollector(void);
	virtual ~SimulationRewardCollector(void);

	void setup(SolverParams& p);
	void addEntry(int currSim, double reward, double expReward);
	void printReward(int currSim);
	void printFinalReward();


	
};
}

#endif

