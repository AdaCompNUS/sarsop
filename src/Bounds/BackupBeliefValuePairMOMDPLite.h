#ifndef BackupBeliefValuePairMOMDPLite_H
#define BackupBeliefValuePairMOMDPLite_H

#include "Backup.h"
#include "BeliefValuePair.h"
using namespace momdp;
namespace momdp 
{
class BackupBeliefValuePairMOMDPLite : public Backup<BeliefValuePair>
{
public:
	BackupBeliefValuePairMOMDPLite(void);
	virtual ~BackupBeliefValuePairMOMDPLite(void);

	virtual SharedPointer<BeliefValuePair> backup(BeliefTreeNode * node);
};
}
#endif

