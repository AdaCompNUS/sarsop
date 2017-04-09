#ifndef SparseVector_H
#define SparseVector_H

#include <vector>
#include <string>
#include <cmath>

#include "MObject.h"
#include "Const.h"
#include "md5wrapper.h"
#include "md5.h"
//#include "Belief.h"  // Commented out during merge on 02102009

using namespace std;
using namespace momdp;

namespace momdp 
{
	// SparseVector_Entry should not inherit from MObject, becuase MObject's size is very significant compared to SparseVector_Entry
	class SparseVector_Entry
	{
	public:
		int index;
		REAL_VALUE value;

		SparseVector_Entry(void) {}
		SparseVector_Entry(int _index, REAL_VALUE _value) :	index(_index), value(_value)
		{}
	};

	class SparseVector : public MObject
	{
		friend class SparseMatrix;
		friend class DenseVector;

	public:
		void copyFrom(const SparseVector& x)
		{
			this->data = x.data;
			this->logicalSize = x.logicalSize;
			this->md5hash = x.md5hash;
		}
		vector< SparseVector_Entry > data;
		void push_back( int index, REAL_VALUE value);

	protected:

		int logicalSize; // the logical length of this sparse vector 



		void add(SparseVector& result,	const SparseVector& x, 	const SparseVector& y);
		void subtract(SparseVector& result,	const SparseVector& x, const SparseVector& y);


		// Hashing 
		string md5hash;

	public:
		SparseVector(void);
		SparseVector(int _size) { resize(_size); }
		virtual ~SparseVector(void);


		// Sealing
		void finalize();


		// Arithmetc
		REAL_VALUE getEntropy();

		int argmax();
		REAL_VALUE operator()( int index) const;
		REAL_VALUE maskedSum( vector<int> mask);
		void copyIndex( vector<int>& result);
		void copyValue( vector<REAL_VALUE>& result);


		void operator+=(const SparseVector& x);
		void operator-=(const SparseVector& x);

		void operator*=(REAL_VALUE s);
		bool operator==(const SparseVector& x) const;


		REAL_VALUE delta(const SparseVector& x) const;
		REAL_VALUE totalDifference(const SparseVector& x) const;
		REAL_VALUE totalHashDifference(const SparseVector& x) const;

		int sampleState() const;
		int argSampleDist() const;

		double norm_inf()
		{
			double val, max = 0.0;
			FOREACH(SparseVector_Entry, xi,  data) 
			{
				val = fabs(xi->value);
				if (val > max) max = val;
			}
			return max;
		}

		REAL_VALUE norm_1()
		{
			double sum = 0.0;
			FOREACH(SparseVector_Entry, xi,  data) 
			{
				sum += fabs(xi->value);
			}
			return sum;
		}

		double norm_2()
		{
			double sum = 0.0;
			FOREACH(SparseVector_Entry, xi,  data) 
			{
				sum += (xi->value)*(xi->value);
			}
			return sum;
		}


		bool isDifferentByAtLeastSingleEntry(const SparseVector& x, const REAL_VALUE& threshold) const;

		// Capacity
		int size(void) const { return logicalSize; }
		int filled(void) const { return data.size(); }

		void resize( int _size);

		//void push_back( int index, REAL_VALUE value);
		//void canonicalize(void) {}


		// IO
		void read(std::istream& in);
		std::ostream& write(std::ostream& out) const;

		//void dumpSparse(std::ostream& out) const;
		//void dumpDense(std::ostream& out, int numStates) const;




		string md5HashValue();
		string ToString() const;

		string convToString(unsigned char *bytes);
		string getHashFromSparseVectorTruncated(SparseVector& x);
		string getHashFromCVector(SparseVector& x);

	};

	// result = x .* y [for all i, result(i) = x(i) * y(i)]
	template <class T, class U> double inner_prod_SparseVector_internal(T xbegin, T xend, U ybegin, U yend)
	{
		double sum = 0.0;
		U yi = ybegin;
		for (T xi = xbegin; xi != xend; xi++) {
			while (1) {
				if (yi == yend) return sum;
				if (yi->index >= xi->index) {
					if (yi->index == xi->index) {
						sum += xi->value * yi->value;
					}
					break;
				}
				yi++;
			}
		}
		return sum;
	}

}


#endif
