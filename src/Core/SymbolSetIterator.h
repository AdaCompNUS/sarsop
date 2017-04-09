#ifndef SymbolSetIterator_H
#define SymbolSetIterator_H

#include <iterator>

#include "Const.h"
using namespace std;
//using namespace momdp;
namespace momdp 
{
template <typename T> class SymbolSet;

template <typename T>
class SymbolSetIterator: public iterator<input_iterator_tag, T>
{
	SymbolSet<T> *parentSet;
	int symbolID;


public:
	SymbolSetIterator(SymbolSet<T> *_parentSet, int _symbolID) 
	{
		parentSet = _parentSet;
		symbolID = _symbolID;
	}

	SymbolSetIterator(const SymbolSetIterator& mit) 
	{
		this->parentSet = mit.parentSet;
		this->symbolID = mit.symbolID;
	}

	SymbolSetIterator<T>& operator++() 
	{
		++symbolID;
		return *this;
	}

	void operator++(int) 
	{
		symbolID++;
	}

	bool operator==(const SymbolSetIterator& rhs) 
	{
		if(parentSet == rhs.parentSet && symbolID == rhs.symbolID)
		{
			return true;
		}
		return false;
	}

	bool operator!=(const SymbolSetIterator& rhs) 
	{
		if(parentSet != rhs.parentSet || symbolID != rhs.symbolID)
		{
			return true;
		}
		return false;
	}

	int operator*() 
	{
		return symbolID;
	}

	T& value() 
	{
		return parentSet->getSymbol(symbolID);
	}
	int index() 
	{
		return symbolID;
	}

};

}


#endif



