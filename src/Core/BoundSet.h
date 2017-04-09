#ifndef BoundSet_H
#define BoundSet_H

#include <vector>
using namespace std;
#include "Bound.h"
#include "BeliefWithState.h"
#include "BeliefTreeNode.h"
#include "FacmodelStructs.h"
using namespace momdp;
namespace momdp 
{
	template <typename T> 
	class BoundSet
	{
	private:

	public:
		BoundSet(void)
		{
		}
		virtual ~BoundSet(void)
		{
		}

		vector<Bound<T> *> set;
		virtual REAL_VALUE getValue(SharedPointer<Belief> belief) = 0;
		virtual void backup(BeliefTreeNode* node) = 0;
	};
}

#endif

