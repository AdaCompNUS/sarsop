#ifndef SparseMatrix_H
#define SparseMatrix_H

#include <vector>
#include <string>
#include <iostream>
#include <cassert>

#include "MObject.h"
#include "SparseVector.h"


using namespace std;
using namespace momdp;
namespace momdp 
{

	class DenseVector;

        class SparseCol 
        {
        public:
            typedef vector<SparseVector_Entry>::const_iterator iterator;
            inline SparseCol(): _begin(NULL), _end(NULL) {}
            inline SparseCol(iterator b, iterator e): _begin(b), _end(e) {}
            inline iterator begin() { return _begin; }
            inline iterator end() { return _end; }
            inline bool empty() { return _begin == _end; }
        private:
            iterator _begin;
            iterator _end;
        };

	class SparseMatrix : public MObject
	{
		friend class SparseVector;
		friend class DenseVector;
	public:
		vector< SparseVector_Entry > data;
		int size1_, size2_;
                vector< int > cols_start;
                vector< int > cols;

	private:
                int colEnd(int index) const;



	public:
		SparseMatrix(void) : size1_(0), size2_(0) 
		{}

		SparseMatrix(int _size1, int _size2) 
		{
			resize(_size1,_size2); 
		}
		virtual ~SparseMatrix(void)
		{
		}

                REAL_VALUE operator()(int r, int c) const;

                SparseCol col(int c) const;

		int size1(void) const { return size1_; }
		int size2(void) const { return size2_; }
		int filled(void) const { return data.size(); }

		void resize(int _size1, int _size2);

		void push_back(int row, int col, REAL_VALUE value);

		// if resize()/push_back() are used to initialize, you must call
		// canonicalize() before performing any operations with the matrix

		void canonicalize(void);

                const vector<int>& nonEmptyColumns() const;
		bool isColumnEmpty(int c) const;

		// Arithmetic
		REAL_VALUE getMaxValue();

		// result = A * x
                void mult(const DenseVector& x, DenseVector& result) const;
	        void mult(const SparseVector& x, DenseVector& result) const;
		DenseVector* mult(const SparseVector& x) const;
		DenseVector* mult(const DenseVector& x) const;

                // result = x * A
	        void leftMult(const DenseVector& x, DenseVector& result) const;
                void leftMult(const SparseVector& x, DenseVector& result) const;

		// IO
		void read(std::istream& in);
		ostream& write(std::ostream& out) const;

	};
}

#endif

