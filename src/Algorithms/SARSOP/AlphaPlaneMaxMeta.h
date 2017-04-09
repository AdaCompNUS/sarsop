/** 
 * The  code is released under GPL v2 
 */



#ifndef ALPHAPLANEMAXMETA_H
#define ALPHAPLANEMAXMETA_H


using namespace std;
namespace momdp{

	/**********************************************
	AlphaPlaneMaxMeta class definition
	Meta infomation about AlphaPlane's max attribute
	**********************************************/
	class AlphaPlaneMaxMeta
	{
	public:
		//fields
		//BeliefTreeNode* belief;
		int cacheIndex;
		int timestamp;
		double lastLB;
	};
};

#endif

