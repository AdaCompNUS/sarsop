#include <algorithm>
#include "SparseMatrix.h"

#include "DenseVector.h"
using namespace momdp;
namespace momdp
{
        REAL_VALUE SparseMatrix::operator()(int r, int c) const
        {
                vector<SparseVector_Entry>::const_iterator  di;

                vector<int>::const_iterator col = lower_bound(cols.begin(), cols.end(), c);
                if (col == cols.end() or *col != c)
                    return 0.0;
                int ci = col - cols.begin();

                vector<SparseVector_Entry>::const_iterator  col_end = data.begin() + colEnd(ci);

                for (di = data.begin() + cols_start[ci]; di != col_end; di++) {
                        if (di->index >= r) {
                                if (di->index == r) {
                                        return di->value;
                                } else {
                                        return 0.0;
                                }
                        }
                }
                return 0.0;
        }

        SparseCol SparseMatrix::col(int c) const
        {
            vector<int>::const_iterator col = lower_bound(cols.begin(), cols.end(), c);
            if (col == cols.end() or *col != c)
                return SparseCol();

            int ci = col - cols.begin();
            vector<SparseVector_Entry>::const_iterator col_start = data.begin() + cols_start[ci];
            vector<SparseVector_Entry>::const_iterator col_end = data.begin() + colEnd(ci);
            return SparseCol(col_start, col_end);
        }

    DenseVector* SparseMatrix::mult(const DenseVector& x) const {
        DenseVector *result = new DenseVector( x.size());
        this->mult(x, *result);
        return result;
    }

	void SparseMatrix::mult(const DenseVector& x, DenseVector& result) const
	{
		// 		SparseVector tmp;
		// 		copy(tmp, x);
		// 		mult(result, A, tmp);
		vector<SparseVector_Entry>::const_iterator  Ai, col_end;

		result.resize( x.size() );

		double xval;

                int mycolIndex = 0;

		FOR(xind, x.size())
		{
                    xval = x.data[xind];
                    while (mycolIndex < cols.size() && cols[mycolIndex] < xind) mycolIndex++;
                    if (mycolIndex == cols.size()) break;
                    if (cols[mycolIndex] == xind) {
			col_end = data.begin() + colEnd(mycolIndex);
			for (Ai = data.begin() + cols_start[mycolIndex]; Ai != col_end; Ai++)
			{
				result(Ai->index) += xval * Ai->value;
			}
		    }
                }

		return;
	}

	DenseVector* SparseMatrix::mult(const SparseVector& x) const {
            DenseVector *result = new DenseVector( x.size());
            this->mult(x, *result);
            return result;
        }

	void SparseMatrix::mult(const SparseVector& x, DenseVector& result) const
	{
		vector<SparseVector_Entry>::const_iterator  Ai, col_end;

		result.resize( x.size() );

		int xind;
		double xval;

                int mycolIndex = 0;

		FOREACH(SparseVector_Entry, xi,  x.data) 
                {
                    xind = xi->index;
                    xval = xi->value;
                    while (mycolIndex < cols.size() && cols[mycolIndex] < xind) mycolIndex++;
                    if (mycolIndex == cols.size()) break;
                    if (cols[mycolIndex] == xind) {
			col_end = data.begin() + colEnd(mycolIndex);
			for (Ai = data.begin() + cols_start[mycolIndex];
				Ai != col_end;
				Ai++)
			{
					result(Ai->index) += xval * Ai->value;
			}
                    }
		}

		return;
	}

	void SparseMatrix::leftMult(const DenseVector& x, DenseVector& result) const
        {
            vector<SparseVector_Entry>::const_iterator  Ai, col_end;

            assert( x.size() == size1() );
            result.resize( size2() );

            FOR (ci, cols.size()) {
                int c = cols[ci];
                col_end = data.begin() + colEnd(ci);
                for (Ai = data.begin() + cols_start[ci]; Ai != col_end; Ai++) {
                    result(c) += x(Ai->index) * Ai->value;
                }
            }
        }

	void SparseMatrix::leftMult(const SparseVector& x, DenseVector& result) const
	{
		assert( x.size() == size1() );
		result.resize( size2() );

		FOR (ci, cols.size()) {
                    int c = cols[ci];
                    result(c) = inner_prod_SparseVector_internal(data.begin() + cols_start[ci],
                                    data.begin() + colEnd(ci), x.data.begin(), x.data.end() );
		}
	}

	void SparseMatrix::resize(int _size1, int _size2)
	{
		size1_ = _size1;
		size2_ = _size2;
                cols.clear();
                cols_start.clear();
		data.clear();
	}

	void SparseMatrix::push_back(int r, int c, REAL_VALUE value)
        {
            // XXX we implicitly assume rows are push_back with increasing order in each column

            data.push_back( SparseVector_Entry( r, value ) );

            if (cols.empty() or cols.back() < c) {
                // new column starts at the data we just added
                cols.push_back(c);
                cols_start.push_back(data.size()-1);
            } else {
                assert(cols.back() == c);
            }
        }

	void SparseMatrix::canonicalize(void)
	{
		//FOR (i, size2_) 
		//{
			//if (col_starts[i] > col_starts[i+1])
			//{
				//col_starts[i+1] = col_starts[i];
			//}
		//}
	}

	void SparseMatrix::read(std::istream& in)
	{
		// Need to be tested		
		int rows, cols;
		int num_entries;
		int r, c;
		REAL_VALUE value;

		in >> rows >> cols;
		resize( rows, cols );

		in >> num_entries;
		FOR (i, num_entries) 
		{
			in >> r >> c >> value;
			push_back( r, c, value );
		}
	}

	std::ostream& SparseMatrix::write(std::ostream& out) const
	{
		out << size1_ << " " << size2_ << std::endl;
		out << data.size() << std::endl;
                FOR (ci, cols.size()) {
                    int c = cols[ci];
                    int col_start = cols_start[ci];
                    int col_end = colEnd(ci);
                    for (int di = col_start; di < col_end; di++) {
                        out << data[di].index << " " << c << " " << data[di].value << std::endl;
                    }
		}
		return out;
	}

	REAL_VALUE SparseMatrix::getMaxValue()
	{
		REAL_VALUE maxVal = data.begin()->value;
		REAL_VALUE val;
		FOREACH(SparseVector_Entry, entry,  data) 
		{
			val = entry->value;
			if(val>maxVal){
				maxVal = val;
			}
		}//end FOR_EACH
		return maxVal;
	}

        const vector<int>& SparseMatrix::nonEmptyColumns() const {
            return cols;
        }

	bool SparseMatrix::isColumnEmpty(int c) const 
	{
		return !binary_search(cols.begin(), cols.end(), c);
	}

        int SparseMatrix::colEnd(int index) const
        {
            int col_end = index+1 < cols.size() ? cols_start[index+1] : data.size();
            return col_end;
        }
}

