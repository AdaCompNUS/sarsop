#ifndef BeliefWithState_H
#define BeliefWithState_H

#include "Belief.h"
#include "MObject.h"
#include "SparseVector.h"
using namespace momdp;
namespace momdp 
{
	class BeliefWithState : public MObject
{
public:
	int sval;
	SharedPointer<SparseVector> bvec;

	BeliefWithState(void);
protected:
	// You should handle all BeliefWithState through smart pointers. Smart pointer will delete this object automatically. You should never manually delete this object
	virtual ~BeliefWithState(void);
};
}


#endif
