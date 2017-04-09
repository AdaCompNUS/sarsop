#ifndef SymbolSet_H
#define SymbolSet_H


// Finite, ordered set
// each symbol has an id

#include <vector>
#include <stdexcept>
#include "SymbolSetIterator.h"
using namespace std;
using namespace momdp;
namespace momdp 
{

template <typename T> 
class SymbolSet
{

private:
	int totalSize;
	vector<T> symbolTable;

	void symbolInputIDSanityCheck(int& id)
	{
		if(id < 0 )
		{
			throw runtime_error("Bug, negative id");
		}
		if(id >= totalSize)
		{
			throw runtime_error("Bug, symbol id exceeds symbol count");
		}
	}

public:
	typedef SymbolSetIterator<T> iterator;

	SymbolSet()
	{
		totalSize = 0;
		symbolTable.clear();
	}

	iterator begin()
	{
		return (iterator(this, 0));
	}

	iterator end()
	{
		return (iterator(this, totalSize));
	}

	virtual int size()
	{
		return symbolTable.size();
	}

	virtual void resize( int newSize)
	{
		totalSize = newSize;
		symbolTable.resize(newSize);
	}

	virtual int add( T& symbol)
	{
		totalSize ++;
		symbolTable.push_back(symbol);
		return symbolTable.size() -1;
	}

	virtual void set(int id, T& symbol)
	{
		symbolInputIDSanityCheck(id);
		symbolTable[id] = symbol;
	}
	virtual T& get(int id)
	{
		symbolInputIDSanityCheck(id);
		return symbolTable[id];
	}
	virtual ~SymbolSet(void)
	{
		symbolTable.clear();
	}
};

}

#endif

