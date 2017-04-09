#include "BeliefCache.h"
#include <sstream>
#include <fstream>
#include <string>
#include <exception>
#include <stdexcept>

using namespace std;

BeliefCache::BeliefCache(void)
{
	lookupTable = new UniqueBeliefHeap();
	currentRowCount = 0;

}

BeliefCache::~BeliefCache(void)
{
}


void BeliefCache::localDelete(SharedPointer<Belief> pointer)
{
	throw runtime_error("not implemented") ;
}

BeliefCacheRow* BeliefCache::getRow(int row) 
{
	validateRowNumber(row);
	return belCache[row];
}

void BeliefCache::validateRowNumber(int row) 
{
	if(row > currentRowCount || row < 0)
	{
		std::ostringstream mystrm;
		mystrm << "Trying to access row = "<<row<<", the total number of rows = "<<
			currentRowCount;
		throw runtime_error(mystrm.str().c_str());
	}
}


int BeliefCache::addBeliefRow( SharedPointer<belief_vector>& bel)
{
	//cout << "BeliefCache::addBeliefRow";
	//bel.write(cout) << endl;
		
	if(!lookupTable->isStorageMaterial(bel))
	{
		std::ostringstream mystrm;
		mystrm << "Trying to add illegal belief <>. Current " <<"belief count = "<<currentRowCount;
		throw runtime_error(mystrm.str().c_str());
	}
	//    std::string hashedVal = bel.hashCode();
	lookupTable->addBeliefRowPair(bel, currentRowCount++);
	BeliefCacheRow* row = new BeliefCacheRow(); // = BeliefCacheRow.createRow();
	row->BELIEF = bel;
	belCache.push_back(row);


	//cout << " as beliefID " << currentRowCount-1 << endl;

	return currentRowCount-1;
}

int BeliefCache::addBeliefRowWithoutCheck( SharedPointer<belief_vector>& bel)
{
	//cout << "BeliefCache::addBeliefRow";
	//bel.write(cout) << endl;
	lookupTable->addBeliefRowPair(bel, currentRowCount++);
	BeliefCacheRow* row = new BeliefCacheRow(); // = BeliefCacheRow.createRow();
	row->BELIEF = bel;
	belCache.push_back(row);
	//cout << " as beliefID " << currentRowCount-1 << endl;

	return currentRowCount-1;
}


bool BeliefCache::hasBelief( SharedPointer<belief_vector>& bel)
{
	return  lookupTable->getBeliefRowIndex(bel) != -1;
}

int BeliefCache::getBeliefRowIndex( SharedPointer<belief_vector>& bel)
{
	return lookupTable->getBeliefRowIndex(bel);
}
ofstream& BeliefCache::writeBeliefVector(std::string fileName)
{
	static ofstream *dumpFile = NULL;
	if(dumpFile == NULL)
	{
		fileName = fileName.erase( fileName.rfind( ".", fileName.length() ) ).append (".bel");
		dumpFile = new ofstream();
		dumpFile->open(fileName.c_str(),  ios::out | ios::trunc);
	}

	return *dumpFile;
}

ofstream& BeliefCache::writeBeliefFunction(std::string fileName)
{
	static ofstream *dumpFile = NULL;
	if(dumpFile == NULL)
	{
		fileName = fileName.erase( fileName.rfind( ".", fileName.length() ) ).append (".bf");
		dumpFile = new ofstream();
		dumpFile->open(fileName.c_str(),  ios::out | ios::trunc);
	}

	return *dumpFile;
}

