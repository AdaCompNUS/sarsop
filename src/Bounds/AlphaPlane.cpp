#include "AlphaPlane.h"
#include "SARSOP.h"
#include <exception>
#include <stdexcept>

using namespace std;


AlphaPlane::AlphaPlane(void) : alpha (new alpha_vector())
{
	
	solverData = new SARSOPAlphaPlaneTuple();
}
SharedPointer<AlphaPlane> AlphaPlane::duplicate()
{
	SharedPointer<AlphaPlane> result ( new AlphaPlane());
	result->alpha = SharedPointer<alpha_vector> (new alpha_vector());
	copy(*result->alpha, *this->alpha);
	result->action = this->action;
	result->sval = this->sval;
	result->solverData = new SARSOPAlphaPlaneTuple();
	return result;
}


AlphaPlane::~AlphaPlane(void)
{
}

void AlphaPlane::init(int _timeStamp, BeliefTreeNode* _birthBelief)
{
	setTimeStamp(_timeStamp);
	SARSOPAlphaPlaneTuple *attachedData = (SARSOPAlphaPlaneTuple *)this->solverData;
	attachedData->certed = 0;//init certed number
	//add the current belief to dominated beliefs of the new plane
	attachedData->certifiedBeliefs.push_back(_birthBelief);
	//add the alpha plane's timestamp on this belief to its timestamp list
	attachedData->certifiedBeliefTimeStamps.push_back(_timeStamp);
}

void AlphaPlane::copyFrom(SharedPointer<alpha_vector> _alpha, int _action, state_val _sval)
{
	this->alpha = _alpha;
	this->action = _action;
	this->sval = _sval;
}
void AlphaPlane::setTimeStamp(int _timeStamp)
{
	timeStamp = _timeStamp;
}

// TODO: Migrate to prunning section
//Functionality:
//	add belief to its dominated list if it's not already in 
void AlphaPlane::addDominatedBelief(int _timeStamp, BeliefTreeNode* _belief)
{
	SARSOPAlphaPlaneTuple *planeTuple = (SARSOPAlphaPlaneTuple *)(this->solverData);
	if(!certDuplicated(_belief))
	{
		//add the current belief to dominated beliefs of the new plane
		planeTuple->certifiedBeliefs.push_back(_belief);
		//add the alpha plane's timestamp on this belief to its timestamp list
		planeTuple->certifiedBeliefTimeStamps.push_back(_timeStamp);

	}
}

bool AlphaPlane::certDuplicated(BeliefTreeNode* n)
{
	SARSOPAlphaPlaneTuple *planeTuple = (SARSOPAlphaPlaneTuple *)(this->solverData);
	int size = (int)planeTuple->certifiedBeliefs.size();
	for(int i=0; i<size; i++){
		BeliefTreeNode* ni = planeTuple->certifiedBeliefs.at(i);
		if (ni==n)
		{
			return true;
		}
	}
	return false;
}

//write this alpha plane to file
void AlphaPlane::write(std::ofstream& out) const
{
    out << "<Vector action=\"" << action <<"\" obsValue=\"" << sval << "\">";
    int n = alpha->size();
    FOR (i, n) 
    {
	out << (*alpha)(i) << " ";
    }
    out << "</Vector>" << endl;
}

void AlphaPlane::writeSparse(std::ofstream& out) const
{
    out << "<SparseVector action=\"" << action <<"\" obsValue=\"" << sval << "\">";
    int n = alpha->size();
    FOR (i, n) 
    {
	if(abs((*alpha)(i))>1e-10){
	    out << "<Entry>";
	    out <<i <<" "<< (*alpha)(i);
	    out << "</Entry>";
	}
    }
    out << "</SparseVector>" << endl;
}

//write this alpha plane to file in the old IV format
/*void AlphaPlane::write(std::ostream& out) const
{
	out << "    {" << endl;
	out << "      action => " << action << "," << endl;
	out << "      observed state val => " << sval << "," << endl;
	out << "      numEntries => " << alpha->size() << "," << endl;
	out << "      entries => [" << endl;
	int n = alpha->size();
	FOR (i, n-1) 
	{
		out << "        " << i << ", " << (*alpha)(i) << "," << endl;
	}
	out << "        " << (n-1) << ", " << (*alpha)(n-1) << endl;

	out << "      ]" << endl;
	out << "    }";
}
*/	
