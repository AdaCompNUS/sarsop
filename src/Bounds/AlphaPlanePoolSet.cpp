#include "Const.h"
#include "AlphaPlanePoolSet.h"
#include "tinyxml.h"
#include <exception>
#include <fstream>
using namespace std;


REAL_VALUE AlphaPlanePoolSet::getValue(SharedPointer<BeliefWithState>& bns)
{
	state_val sval = bns->sval;
	double bestVal;
	bestVal = set[sval]->getValue( bns->bvec);

	return bestVal;
}

// TODO : this is for the first time visited belief only
SharedPointer<AlphaPlane> AlphaPlanePoolSet::getValueAlpha(SharedPointer<BeliefWithState>& bns)
{
	state_val sval = bns->sval;
	return set[sval]->getValueAlpha( bns->bvec);
}

SharedPointer<AlphaPlane> AlphaPlanePoolSet::backup(BeliefTreeNode * node)
{
	SharedPointer<AlphaPlane> result = backupEngine->backup(node);
	for(int i = 0 ; i < onBackup.size(); i++)
	{
		(*onBackup[i])(solver, node, result);
	}
	return result;
}


SharedPointer<AlphaPlane> AlphaPlanePoolSet::getBestAlphaPlane(BeliefWithState& beliefandState)
{
	state_val sval = beliefandState.sval;
	SharedPointer<AlphaPlane> bestAlp;
	bestAlp =  set[sval]->getBestAlphaPlane(beliefandState.bvec);
	return bestAlp;

}


SharedPointer<AlphaPlane> AlphaPlanePoolSet::getBestAlphaPlane(BeliefTreeNode& cn)
{
	state_val sval = cn.s->sval;
	SharedPointer<AlphaPlane> bestAlp;

	bestAlp =  set[sval]->getBestAlphaPlane(cn);
	return bestAlp;
}

SharedPointer<AlphaPlane> AlphaPlanePoolSet::getBestAlphaPlane1(BeliefWithState& beliefandState)
{
	state_val sval = beliefandState.sval;
	SharedPointer<AlphaPlane>bestAlp;
	bestAlp =  set[sval]->getBestAlphaPlane1(beliefandState.bvec);
	return bestAlp;
}

//write out policy in new xml format
void AlphaPlanePoolSet::writeToFile(const std::string& outFileName, string problemName) 
{
	ofstream out(outFileName.c_str());
	if (!out) 
	{
		cerr << "ERROR: Bounds::writeToFile: couldn't open " << outFileName << " for writing " << endl;
		exit(EXIT_FAILURE);
	}
	
	string declaration = 
	"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n<Policy version=\"0.1\" type=\"value\" model=\""+ problemName +"\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"policyx.xsd\">";
	out << declaration << endl ;
	int totalnumPlanes = 0;
	FOR (sval, set.size()) 
	{
		totalnumPlanes += set[sval]->planes.size();
	}
	int vectorLength = problem->YStates->size();
	int numObsValue = set.size();
	out << "<AlphaVector vectorLength=\"" << vectorLength << "\" numObsValue=\"" << numObsValue << "\" numVectors=\"" << totalnumPlanes << "\">" << endl;
	FOR (sval, set.size())
	{
		list<SharedPointer<AlphaPlane> >::const_iterator pi = set[sval]->planes.begin();
		FOR (i, set[sval]->planes.size()) 
		{
			(*pi)->write(out);
			pi++;
			    
		}
	}
	out << "</AlphaVector> </Policy>" << endl;
}

//write out policy in old format
/*
void AlphaPlanePoolSet::writeToFile(const std::string& outFileName) 
{
	ofstream out(outFileName.c_str());
	if (!out) 
	{
		cerr << "ERROR: Bounds::writeToFile: couldn't open " << outFileName << " for writing " << endl;
		exit(EXIT_FAILURE);
	}

	out <<
		"# This file is a POMDP policy, represented as a set of \"lower bound\n"
		"# planes\", each of which consists of the value of the observed state\n"
		"# variable, an alpha vector and a corresponding action.\n"
		"# Given a particular value of the observed state variable x\n"
		"# and a belief b, this information can be used to answer two queries\n"
		"# of interest:\n"
		"#\n"
		"#   1. What is a lower bound on the expected long-term reward starting\n"
		"#        from state value x and belief b?\n"
		"#   2. What is an action that achieves that expected reward lower bound?\n"
		"#\n"
		"# Each lower bound plane is only defined over a subset of the belief\n"
		"# simplex--it is defined for those beliefs b such that the non-zero\n"
		"# entries of b are a subset of the entries present in the plane's alpha\n"
		"# vector.  If this condition holds we say the plane is 'applicable' to b.\n"
		"#\n"
		"# Given a belief b, both of the queries above can be answered by the\n"
		"# following process: first, throw out all the planes that are not\n"
		"# applicable to b.  Then, for each of the remaining planes, take the inner\n"
		"# product of the plane's alpha vector with b.  The highest inner product\n"
		"# value is the expected long-term reward lower bound, and the action label\n"
		"# for that plane is the action that achieves the bound.\n"
		"\n"
		;
	out << "{" << endl;
	out << "  policyType => \"MaxPlanesLowerBoundWithObservedState\"," << endl;

	int totalnumPlanes = 0;
	FOR (sval, set.size()) 
	{
		totalnumPlanes += set[sval]->planes.size();
	}

	out << "  numPlanes => " << totalnumPlanes << "," << endl;
	out << "  planes => [" << endl;

	FOR (sval, set.size()-1)
	{
		list<SharedPointer<AlphaPlane> >::const_iterator pi = set[sval]->planes.begin();
		FOR (i, set[sval]->planes.size()) 
		{
			(*pi)->write(out);
			out << "," << endl;
			pi++;
		}
	}

	list<SharedPointer<AlphaPlane> >::const_iterator pi = set.back()->planes.begin();

	FOR (i, set.back()->planes.size()-1) 
	{
		(*pi)->write(out);
		out << "," << endl;
		pi++;
	}
	(*pi)->write(out);
	out << endl;

	out << "  ]" << endl;
	out << "}" << endl;

	out.close();
}
*/
