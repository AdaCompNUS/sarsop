#include "BackupAlphaPlaneMOMDPLite.h"
#include "exception" 
#include <stdexcept>


using namespace std;
BackupAlphaPlaneMOMDPLite::BackupAlphaPlaneMOMDPLite(void)
{
}

BackupAlphaPlaneMOMDPLite::~BackupAlphaPlaneMOMDPLite(void)
{
}
SharedPointer<AlphaPlane> BackupAlphaPlaneMOMDPLite::backup(BeliefTreeNode * node)
{
	throw runtime_error("not implemented");
}