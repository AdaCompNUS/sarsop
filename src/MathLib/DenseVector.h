#ifndef DenseVector_H
#define DenseVector_H

#include <vector>
#include <string>
using namespace std;

#include "MObject.h"
#include "Const.h"
#include "SparseVector.h"
#include "SparseMatrix.h"


using namespace momdp;
namespace momdp 
{
class SparseVector;

class DenseVector :	public MObject
{
	friend class SparseMatrix;
	friend class SparseVector;
public:
	vector<REAL_VALUE> data;

protected:
	

public:
	DenseVector(void);
	DenseVector(int _size);

	void copyFrom(const DenseVector& x)
	{
		this->data = x.data;
	}
	virtual ~DenseVector(void);

	// Accessor
	REAL_VALUE& operator()(unsigned int i) 
	{ 
		return data[i]; 
	}

	REAL_VALUE operator()(unsigned int i) const 
	{ 
		return data[i]; 
	}

	// Arithmetic functions 
	void operator*=(REAL_VALUE s);
	void operator+=(const DenseVector& x);
	void operator-=(const DenseVector& x);
	
	void operator+=(const SparseVector& x);
	void operator-=(const SparseVector& x);
	
	int argSampleDist();
	double norm_inf()
	{
		double val, max = 0.0;
		FOREACH(double, xi,  data) 
		{
			val = fabs(*xi);
			if (val > max) max = val;
		}
		return max;
	}

	double norm_2()
	{
		double sum = 0.0;
		FOR(i, data.size())
		{
			sum += data[i]*data[i];
		}
		return sum;
	}



	DenseVector* mult( SparseMatrix& A);

	// Capacity
	int size(void) const 
	{ 
		return data.size(); 
	}

	void resize(int _size, REAL_VALUE value);
	void resize(int _size);



	// Input Output Functions:
	void read(std::istream& in);
	std::ostream& write(std::ostream& out) const;

	string ToString() const;
};

}


#endif
