#ifndef IndexedTuple_H
#define IndexedTuple_H

#include <vector>
#include <stdexcept>


using namespace std;
using namespace momdp;
namespace momdp 
{
template <typename T>
class IndexedTuple
{
private:
	vector<T> tupleTable;

public:
	IndexedTuple(void)
	{
	}
	virtual ~IndexedTuple(void)
	{
	}

	T& set(size_t index) 
	{
		if(index < 0)
		{
			throw runtime_error("bug, index < 0 ");
		}
		if(index >= tupleTable.size())
		{
			// expand the table
			tupleTable.resize(index+1);
		}
		return tupleTable[index];
	}
	const T& get(size_t index) 
	{
		if(index < 0)
		{
			throw runtime_error("bug, index < 0 ");
		}
		if(index >= tupleTable.size())
		{
			// expand the table
			throw runtime_error("bug, index > size ");
		}

		return tupleTable[index];
	}


};

}

#endif

