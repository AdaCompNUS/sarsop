#ifndef MOMDPLite_H
#define MOMDPLite_H

#include <string>

#include "MOMDP.h"
#include "StateTransitionY.h"
using namespace std;
using namespace momdp;
namespace momdp 
{
class MOMDPLite : public MOMDP
{
public:
	MOMDPLite(void);
	virtual ~MOMDPLite(void);

	StateTransitionY* YTrans;

	static MOMDPLite* LoadProbem(string problemName)
	{
		return NULL;
	}
};
}
#endif
