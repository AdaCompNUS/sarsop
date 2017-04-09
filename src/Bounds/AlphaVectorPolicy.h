#ifndef AlphaVectorPolicy_H
#define AlphaVectorPolicy_H

#include <vector>
#include <string>
#include <iostream>
#include "MOMDP.h"
#include "MObject.h"
using namespace std;
using namespace momdp;

namespace momdp
{
	class AlphaPlanePoolSet;

	class AlphaVectorPolicy : public MObject
	{
	private:
		SharedPointer<MOMDP> problem;
		AlphaPlanePoolSet* alphaPlanePoolSet;
		string policyFile;
        int valueAction;
	public:
		AlphaVectorPolicy(SharedPointer<MOMDP> problem);
		int getBestActionLookAhead(BeliefWithState& b);
		int getBestActionLookAhead(BeliefWithState& b, REAL_VALUE& maxValue);
		int getBestAction(BeliefWithState& b);
		int getBestAction(BeliefWithState& b, REAL_VALUE& maxValue);

		int getBestActionLookAhead(vector<belief_vector>& b, DenseVector& belX); // SYL07282010 - modify function so that it follows RSS09 paper. Input to function is b_x and b_{y|x}.
		int getBestActionLookAhead(SharedPointer<belief_vector>& b, DenseVector& belX); // SYL07282010 - modify function so that it follows RSS09 paper. Input to function is b_x and b_y.

		int getBestAction(SharedPointer<belief_vector>& b, DenseVector& belX);  // Input to function is b_x and b_y.
		int getBestAction(vector<belief_vector>& b, DenseVector& belX);  // SYL07282010 - added the counterpart to int getBestAction(SharedPointer<belief_vector>& b, DenseVector& belX). Input to function is b_x and b_{y|x}.

		int getBestActionLookAhead_alternative(vector<belief_vector>& b, DenseVector& belX);  // SYL07282010 replaced with code which follows RSS09 paper. Input to function is b_x and b_{y|x}.
		int getBestActionLookAhead_alternative(SharedPointer<belief_vector>& b, DenseVector& belX);  // SYL07282010 replaced with code which follows RSS09 paper. Input to function is b_x and b_y.

		
		bool readFromFile(const std::string& inFileName);
        int getValueAction();
	};
}

#endif

