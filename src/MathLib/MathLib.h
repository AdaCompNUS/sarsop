#ifndef MathLib_H
#define MathLib_H

#include <algorithm>
#include <cstdlib>
#include <cmath>

#include "mdpCassandra.h"
#include "pomdpCassandraWrapper.h"

#include "SparseVector.h"
#include "SparseMatrix.h"
#include "DenseVector.h"

using namespace std;
using namespace momdp;
namespace momdp 
{

	class MathLib
	{
	public:
		MathLib(void);
		~MathLib(void);
	};


	inline REAL_VALUE unit_rand(void)
	{
		return ((REAL_VALUE)rand())/RAND_MAX;
	}

	inline REAL_VALUE BeliefEntryTruncate(REAL_VALUE input)
	{
		input *= 10^BELIEF_DECIMAL_PRECISION;
		input += 0.5;
		input = floor(input);
		return input;
	}

	struct kmatrix_entry {
		unsigned int r, c;
		double value;

		kmatrix_entry(void) {}
		kmatrix_entry(unsigned int _r,
			unsigned int _c,
			double _value) :
		r(_r),
			c(_c),
			value(_value)
		{}
	};

	struct kmatrix
	{
		unsigned int size1_, size2_;
		std::vector< kmatrix_entry > data;

		kmatrix(void) : size1_(0), size2_(0) {}
		kmatrix(unsigned int _size1, unsigned int _size2) { resize(_size1,_size2); }

		double operator()(unsigned int r, unsigned int c) const;

		unsigned int size1(void) const { return size1_; }
		unsigned int size2(void) const { return size2_; }
		unsigned int filled(void) const { return data.size(); }
		void clear(void);
		void resize(unsigned int _size1, unsigned int _size2, double value = 0.0);
		void push_back(unsigned int r, unsigned int c, double value);
		void canonicalize(void);

		void read(std::istream& in);
	};


	// result = x
	inline void copy(SparseVector& result, const SparseVector& x) 
	{
		result.copyFrom(x);
	}
	inline void copy(DenseVector& result, const DenseVector& x) 
	{ 
		// copy all the data
		result.copyFrom(x);
	}

	// result = x
	void copy(DenseVector& result, const SparseVector& x);

	// result = x
	void copy(SparseVector& result, const DenseVector& x);

	// result = A (side-effect: canonicalizes A)
	void copy(SparseMatrix& result, kmatrix& A);

	// result = A(.,c)
	void copy_from_column(SparseVector& result, const SparseMatrix& A, unsigned int c);

	// result = A(:,c)
	void copy_from_column(DenseVector& result, const SparseMatrix& A, unsigned int c);
	// A(r,c) = v
	void kmatrix_set_entry(kmatrix& A, unsigned int r, unsigned int c,
		double v);

	// A = A'
	void kmatrix_transpose_in_place(kmatrix& A);


	// result = A * x
	void mult(DenseVector& result, const SparseMatrix& A, const SparseVector& x);

	void mult(DenseVector& result, const SparseMatrix& A, const DenseVector& x);


	void mult(SparseVector& result, const SparseMatrix& A, const SparseVector& x);


	// result = x * A
	void mult(DenseVector& result, const DenseVector& x, const SparseMatrix& A);



	void mult(DenseVector& result, const SparseVector& x, const SparseMatrix& A);

	void mult(SparseVector& result, const SparseVector& x, const SparseMatrix& A);


	// result = x .* y [for all i, result(i) = x(i) * y(i)]
	void emult(DenseVector& result, const DenseVector& x, const DenseVector& y);


	// result = x .* y [for all i, result(i) = x(i) * y(i)]
	void emult(SparseVector& result, const SparseVector& x, const SparseVector& y);

	// result = A(:,c) .* x
	void emult_column(SparseVector& result, const SparseMatrix& A, unsigned int c, const SparseVector& x);

	// result = x .* y
	void emult(DenseVector& result, const DenseVector& x, const SparseVector& y);

	// result = A(:,c) .* x
	void emult_column(DenseVector& result, const SparseMatrix& A, unsigned int c, const DenseVector& x);

	// return x' * y
	double inner_prod(const DenseVector& x, const SparseVector& y);

	// return x' * y
	double inner_prod(const SparseVector& x, const SparseVector& y);

	double inner_prod_binary(const SparseVector& x, const SparseVector& y);
	int binarySearch(const SparseVector &x, int key);
	int quickLog2(int n);

	// result = max(result,x)
	void max_assign(DenseVector& result, const DenseVector& x);
	// return A(:,c)' * y
	double inner_prod_column(const SparseMatrix& A, unsigned int c, const SparseVector& y);


	// return true if for all i: x(i) >= y(i) - eps
	bool dominates(const DenseVector& x, const DenseVector& y, double eps);

	// return true if for all i: x(i) >= y(i) - eps
	bool dominates(const SparseVector& x, const SparseVector& y, double eps);


	// result = A
	void copy(SparseMatrix& result, CassandraMatrix A, int numColumns);

	// result = A
	void copy(kmatrix& result, CassandraMatrix A, int numColumns);

	/**********************************************************************
	* FUNCTIONS
	**********************************************************************/

	// result = A
	inline void copy(SparseMatrix& result, CassandraMatrix A, int numColumns)
	{
		kmatrix B;
		copy(B, A, numColumns);
		copy(result, B);
	}

	// result = A
	inline void copy(kmatrix& result, CassandraMatrix A, int numColumns)
	{
		result.resize(A->num_rows, numColumns);
		FOR (r, A->num_rows) {
			int rowOffset = A->row_start[r];
			int rowSize = A->row_length[r];
			FOR (i, rowSize) {
				int j = rowOffset + i;
				int c = A->col[j];
				double val = A->mat_val[j];
				result.push_back(r, c, val);
			}
		}
		result.canonicalize();
	}


	// Set all entries to zero.
	//void set_to_zero(dmatrix& M);
	void set_to_zero(kmatrix& M);
	void set_to_zero(SparseMatrix& M);
	void set_to_zero(DenseVector& v);
	void set_to_zero(SparseVector& v);

	/**********************************************************************
	* FUNCTIONS
	**********************************************************************/

	// Set all entries to zero.
	/*inline void set_to_zero(dmatrix& M)
	{
	M.resize( M.size1(), M.size2() );
	}*/

	inline void set_to_zero(kmatrix& M)
	{
		M.resize( M.size1(), M.size2() );
	}

	inline void set_to_zero(SparseMatrix& M)
	{
		M.resize( M.size1(), M.size2() );
	}

	inline void set_to_zero(DenseVector& v)
	{
		v.resize( v.size() );
	}

	inline void set_to_zero(SparseVector& v)
	{
		v.resize( v.size() );
	}

	// Index of maximum element of a vector
	int argmax_elt(const DenseVector& v);
	int argmax_elt(const SparseVector& v);

	// b represents a discrete probability distribution Pr(outcome = i) = b(i).
	// Chooses an outcome according to the distribution.
	inline int chooseFromDistribution(const DenseVector& b) 
	{
		double r = unit_rand();
		FOR (i, b.size())
		{
			r -= b(i);
			if (r <= 0) 
			{
				return i;
			}
		}
		return b.data.size()-1;
	}

	inline int chooseFromDistribution(const SparseVector& b) {
		double r = unit_rand();
		int lastIndex = 0;
		FOR_CV(b) 
		{
			r -= CV_VAL(b);
			lastIndex = CV_INDEX(b);
			if (r <= 0)
			{
				return lastIndex ;
			}
		}
		return lastIndex;
	}

	inline int chooseFromDistribution(const SparseVector& b, double r)
	{
		int lastIndex = 0;
		FOR_CV(b) 
		{
			r -= CV_VAL(b);
			lastIndex = CV_INDEX(b);
			if (r <= 0)
			{
				return lastIndex;
			}
		}
		return lastIndex;
	}


	// TODO :: Remove kmatrix implementation
}

#endif


