#include "DenseVector.h"
#include "SparseVector.h"
#include "SparseMatrix.h"

#include <iostream>
#include <cassert>
#include <sstream>
#include "MathLib.h"

using namespace std;
using namespace momdp;
namespace momdp
{
DenseVector::DenseVector(void)
{
}
DenseVector::DenseVector(int _size)
{ 
	resize(_size); 
}
DenseVector::~DenseVector(void)
{
}

// Arithmetic

// TODO dead code, should be ok to remove
//DenseVector* DenseVector::mult( SparseMatrix& A)
	//{
		//vector<SparseVector_Entry>::const_iterator  Ai, col_end;
		//assert( size() == A.size1() );
		
		//DenseVector* result = new DenseVector( A.size2() );

		//FOR (c, A.size2()) 
		//{
			//col_end = A.data.begin() + A.col_starts[c+1];
			//for (Ai = A.data.begin() + A.col_starts[c];
				//Ai != col_end;
				//Ai++) 
			//{
					//(*result)(c) += (*this)(Ai->index) * Ai->value;
			//}
		//}

		//return result;
	//}

#define DenseVector_Operator( OP )							\
	void DenseVector::operator OP(const DenseVector& x)			\
{															\
	vector<REAL_VALUE>::const_iterator  xi;                 \
	assert( size() == x.size() );							\
	xi = x.data.begin();									\
	FOREACH_NOCONST(REAL_VALUE, di,  data) {                    \
	(*di) OP (*xi);										\
	xi++;												\
}														\
}


DenseVector_Operator( += );
DenseVector_Operator( -= );

// TODO: += -= for sparse vector

void DenseVector::operator*=(REAL_VALUE s)
{
	FOREACH_NOCONST(REAL_VALUE, di,  data) {
		(*di) *= s;
	}
}

int DenseVector::argSampleDist()
{
	REAL_VALUE randNumber = unit_rand();
	REAL_VALUE sum = 0.0;
	int state = 0;
	FOREACH(REAL_VALUE, entry,  data) 
	{
		sum += *entry;
		if(randNumber < sum )
		{
			return state; 
		}
		state ++;
	}
	return state-1;
}



// capacity
void DenseVector::resize(int _size, REAL_VALUE value)
{
	assert(0 == value);
	data.resize( _size );
	FOREACH_NOCONST(double, di,  data) 
	{
		(*di) = value;
	}
	
}

void DenseVector::resize(int _size)
{
	resize(_size, 0.0);
}

// IO

void DenseVector::read(istream& in)
{
	int num_entries;

	in >> num_entries;
	resize( num_entries );
	FOR (i, num_entries) 
	{
		in >> data[i];
	}

#if 0
	std::cout << "dense read: size = " << data.size() << std::endl;
	FOR (i, data.size()) {
		if (i == 10) {
			std::cout << "..." << std::endl;
			break;
		}
		std::cout << i << " " << data[i] << std::endl;
	}
#endif
}

ostream& DenseVector::write(ostream& out) const
{
	out << size() << std::endl;
	FOREACH(REAL_VALUE, x,  data) 
	{
		out << (*x) << " ";
	}
	return out;
}


string DenseVector::ToString() const
{
	stringstream out;
	out << "[";
	FOR(i, data.size())
	{
		out << data[i];
		if ( i != data.size()-1 )
			out << ",";
	}
	out << "]";
	return out.str();
}

}

