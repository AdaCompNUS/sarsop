#include "MathLib.h"

MathLib::MathLib(void)
{
}

MathLib::~MathLib(void)
{
}
using namespace momdp;
namespace momdp
{

	void copy(DenseVector& result, const SparseVector& x)
	{
		result.resize( x.size() );
		FOREACH(SparseVector_Entry, xi,  x.data) 
		{
			result.data[xi->index] = xi->value;
		}
	}


	void copy(SparseVector& result, const DenseVector& x)
	{
		int num_filled;
		int i;
		vector<SparseVector_Entry>::iterator  ri;


		// count non-zeros
		num_filled = 0;
		FOREACH(double, xi,  x.data) {
			if (fabs(*xi) > SPARSE_EPS) num_filled++;
		}
		//std::cout << "convert: num_filled=" << num_filled << std::endl;

		// resize result vector
		result.resize( x.size() );
		result.data.resize(num_filled);

		// copy non-zeros to result
		i = 0;
		ri = result.data.begin();
		int rCount = 0;
		FOREACH(double, xi,  x.data) {
			if (fabs(*xi) > SPARSE_EPS) {
				assert(rCount <= num_filled);
				ri->index = i;
				ri->value = *xi;
				ri++;
				rCount++;
			}
			i++;
		}
	}

	void copy(SparseMatrix& result, kmatrix& A)
	{
		A.canonicalize();
		result.resize( A.size1(), A.size2() );
		FOREACH(kmatrix_entry, entry,  A.data) {
			if (fabs(entry->value) > SPARSE_EPS) {
				result.push_back( entry->r, entry->c, entry->value );
			}
		}
		result.canonicalize();
	}

	// result = A(:,c)
	void copy_from_column(SparseVector& result, const SparseMatrix& A,
		unsigned int c)
	{
		assert( 0 <= c && c < A.size2() );

		vector<SparseVector_Entry>::const_iterator  Ai;

		vector<SparseVector_Entry>::iterator  ri;


                SparseCol col = A.col(c);

		result.resize( A.size1() );
		result.data.resize( col.end() - col.begin() );
		for (Ai = col.begin(), ri=result.data.begin(); Ai != col.end(); Ai++, ri++) {
			(*ri) = (*Ai);
		}
	}

	// result = A(:,c)
	void copy_from_column(DenseVector& result, const SparseMatrix& A,
		unsigned int c)
	{
		assert( 0 <= c && c < A.size2() );
		vector<SparseVector_Entry>::const_iterator  Ai, col_end;


		result.resize( A.size1() );

                SparseCol col = A.col(c);
		for (Ai = col.begin(); Ai != col.end(); Ai++) {
			result(Ai->index) = Ai->value;
		}
	}

	// A(r,c) = v
	void kmatrix_set_entry(kmatrix& A, unsigned int r, unsigned int c,
		double v)
	{
		A.push_back( r, c, v );
	}

	// A = A'
	void kmatrix_transpose_in_place(kmatrix& A)
	{
		std::swap(A.size1_, A.size2_);
		FOREACH_NOCONST(kmatrix_entry, Ai,  A.data) {
			std::swap( Ai->r, Ai->c );
		}
		A.canonicalize();
	}

	// result = A * x
	void mult(DenseVector& result, const SparseMatrix& A, const SparseVector& x)
	{
            A.mult(x, result);
	}

	void mult(DenseVector& result, const SparseMatrix& A, const DenseVector& x)
	{
            A.mult(x, result);
	}

	// result = A * x
	void mult(SparseVector& result, const SparseMatrix& A, const SparseVector& x)
	{
		DenseVector tmp;

		mult( tmp, A, x );
		copy( result, tmp );
	}

	// result = x * A
	void mult(DenseVector& result, const DenseVector& x, const SparseMatrix& A)
	{
            A.leftMult(x, result);
	}

	// result = x * A

	void mult(DenseVector& result, const SparseVector& x, const SparseMatrix& A)
	{
            A.leftMult(x, result);
	}

	// result = x * A
	void mult(SparseVector& result, const SharedPointer<SparseVector> x, const SharedPointer<SparseMatrix>  A)
	{
		mult(result, *x, *A);
	}
	void mult(SparseVector& result, const SharedPointer<SparseVector> x, const SparseMatrix& A)
	{
		mult(result, *x, A);
	}
	void mult(SparseVector& result, const SparseVector& x, const SharedPointer<SparseMatrix>  A)
	{
		mult(result, x, *A);
	}
	void mult(SparseVector& result, const SparseVector& x, const SparseMatrix& A)
	{
		DenseVector tmp;
		mult(tmp,x,A);
		copy(result,tmp);
	}

	// result = x .* y [for all i, result(i) = x(i) * y(i)]
	void emult(DenseVector& result, const DenseVector& x, const DenseVector& y)
	{
		assert( x.size() == y.size() );
		result.resize( x.size() );
		FOR (i, result.size()) {
			result(i) = x(i) * y(i);
		}
	}

	// result = x .* y [for all i, result(i) = x(i) * y(i)]
	template <class T, class U> void emult_cc_internal(SparseVector& result, T xbegin, T xend,  U ybegin, U yend)
	{
		U yi = ybegin;
		for (T xi = xbegin; xi != xend; xi++) {
			while (1) {
				if (yi == yend) return;
				if (yi->index >= xi->index) {
					if (yi->index == xi->index) {
						result.push_back( xi->index, xi->value * yi->value);
					}
					break;
				}
				yi++;
			}
		}
	}

	// result = x .* y [for all i, result(i) = x(i) * y(i)]
	void emult(SparseVector& result, const SparseVector& x, const SparseVector& y) {
		assert( x.size() == y.size() );
		result.resize( x.size() );

		emult_cc_internal( result, x.data.begin(), x.data.end(),
			y.data.begin(), y.data.end() );

		//result.finalize();
	}

	// result = A(:,c) .* x
	void emult_column(SparseVector& result, const SparseMatrix& A, unsigned int c, const SparseVector& x)
	{
		assert( A.size1() == x.size() );
		assert( 0 <= c && c < A.size2() );
		result.resize( x.size() );

                SparseCol col = A.col(c);

		emult_cc_internal( result,
                        col.begin(), col.end(),
			x.data.begin(), x.data.end() );

		//result.finalize();
	}

	// result = x .* y [for all i, result(i) = x(i) * y(i)]
	template <class T> void emult_dc_internal(DenseVector& result, const DenseVector& x, T ybegin, T yend)
	{
		int yind;
		for (T yi = ybegin; yi != yend; yi++) {
			yind = yi->index;
			result(yind) = x(yind) * yi->value;
		}
	}

	// result = x .* y
	void emult(DenseVector& result, const DenseVector& x, const SparseVector& y)
	{
		assert( x.size() == y.size() );
		result.resize( x.size() );
		emult_dc_internal( result, x, y.data.begin(), y.data.end() );
	}

	// result = A(:,c) .* x
	void emult_column(DenseVector& result, const SparseMatrix& A, unsigned int c, const DenseVector& x)
	{
		assert( A.size1() == x.size() );
		assert( 0 <= c && c < A.size2() );
		result.resize( x.size() );
                SparseCol col = A.col(c);
		emult_dc_internal( result, x, col.begin(), col.end());
	}

	// return x' * y
	double inner_prod(const DenseVector& x, const SparseVector& y)
	{
		assert( x.size() == y.size() );
		double sum = 0.0;
		FOREACH(SparseVector_Entry, yi,  y.data) 
		{
			sum += x(yi->index) * yi->value;
		}
		return sum;
	}
	// return x' * y
	double inner_prod(const SparseVector& x, const SparseVector& y)
	{
		if(x.size()!= y.size())
			printf("x size is : %d, y size is %d\n", (int)x.size(), (int)y.size());
		assert( x.size() == y.size() );
		double result;

		//check whether binary search is useful
		if(x.data.size()>=y.data.size() && (quickLog2(x.data.size())*y.data.size())< (x.data.size())){
			//		  printf("x size:%d, y size:%d\n", (int)x.data.size(), (int)y.data.size());
			result = inner_prod_binary(x, y);
		}
		else if(y.data.size()>x.data.size() && (quickLog2(y.data.size())*x.data.size())<(y.data.size())){
			//		  printf("x size:%d, y size:%d\n", (int)x.data.size(), (int)y.data.size());
			result = inner_prod_binary(y, x);
		}
		else{
			result = inner_prod_SparseVector_internal( x.data.begin(), x.data.end(),
				y.data.begin(), y.data.end() );
		}
		return result;
	}
	//quick log making use of shifts
	int quickLog2(int n){
		int r = 0;
		while(n!= 0){
			n = n >> 1;
			r++;
		}
		return r;
	}

	//Function: binarySearch
	//Functionality:
	//	to find the position of entry in x which has index value as 'key'
	//Parameter:
	//	key:	the index value to be found
	//Returns:
	//	the position of entry, if it is found
	//	-1, else
	int binarySearch(const SparseVector &x, int lowerbound, int key)
	{
		int position;
		//	   int lowerbound = 0;
		int upperbound = x.data.size()-1;
		// calculate the first search position.
		position = ( lowerbound + upperbound)>>1;

		/*		if (position < 0 || position >= (int)x.data.size())
		return -1;*/
		// unnecessary condition, commented out by Yanzhu

		while(((int)((x.data.at(position).index))!= key) && (lowerbound <= upperbound))
		{
			if (((int)(x.data.at(position).index)) > key)               // if the value in the array[position]..
			{                                                       // is greater than the number for ��?				upperbound = position - 1;    // which we are searching, change ..
			}                                                       // upperbound to the position��?			else                                                 // minus one.
			{                                                        // Else, change lowerbound to ..
				lowerbound = position + 1;     // position plus one.
			}
			position = (lowerbound + upperbound)>>1;
			if (position < 0 || position >= (int)x.data.size())
				break;

		}

		/*		if (lowerbound <= upperbound){
		return position;
		}
		return -1;*/
		// unnecessary checking, commented out by Yanzhu
		return position;

	}

	//added!! to improve efficiency of inner product
	//rn 11/30/2006
	// result = x .* y [for all i, result(i) = x(i) * y(i)]
	double inner_prod_binary(const SparseVector& x, const SparseVector& y)
	{
		double sum = 0.0;
		int xi_position;
		SparseVector_Entry xi;
		vector<SparseVector_Entry>::const_iterator  yi;

		xi_position = 0;
		for (yi = y.data.begin(); yi!=y.data.end() && xi_position<(int)x.data.size(); yi++)
		{
			xi = x.data.at(xi_position);
			if (xi.index == yi->index){
				sum+= xi.value*yi->value;
			}
			else{
				//find data in x where index = yi_index
				xi_position = binarySearch(x, xi_position, yi->index);

				if(xi_position> -1){
					xi = x.data.at(xi_position);//@
					//			  printf("result of binary search: xi_index:%d, yi_index:%d\n", xi.index, yi->index);
					sum += xi.value * yi->value;
				}
			}
			xi_position++;
		}

		return sum;
	}
	//end added

	// return A(:,c)' * x
	double inner_prod_column(const SparseMatrix& A, unsigned int c, const SparseVector& x)
	{
		assert( A.size1() == x.size() );
		assert( 0 <= c && c < A.size2() );
                SparseCol col = A.col(c);
		return inner_prod_SparseVector_internal( col.begin(), col.end(), x.data.begin(), x.data.end() );
	}

	// return true if for all i: x(i) >= y(i) - eps
	bool dominates(const DenseVector& x, const DenseVector& y, double eps)
	{
		FOR (i, x.size()) {
			if (x(i) < y(i) - eps) return false;
		}
		return true;
	}

	// return true if for all i: x(i) >= y(i) - eps
	bool dominates(const SparseVector& x, const SparseVector& y, double eps)
	{
		vector<SparseVector_Entry>::const_iterator  xi, xend;

		vector<SparseVector_Entry>::const_iterator  yi, yend;

		unsigned int xind, yind;
		bool xdone = false, ydone = false;

		assert( x.size() == y.size() );

#define CHECK_X() \
	if (xi == xend) { \
	xdone = true; \
	goto main_loop_done; \
	} else { \
	xind = xi->index; \
	}

#define CHECK_Y() \
	if (yi == yend) { \
	ydone = true; \
	goto main_loop_done; \
	} else { \
	yind = yi->index; \
	}

		xi = x.data.begin();
		yi = y.data.begin();
		xend = x.data.end();
		yend = y.data.end();

		CHECK_X();
		CHECK_Y();

		while (1) {
			if (xind < yind) {
				// x(xind) == xi->value, y(xind) == 0
				if (xi->value < -eps) return false;
				xi++;
				CHECK_X();
			} else if (xind == yind) {
				// x(xind) == xi->value, y(xind) == yi->value
				if (xi->value < yi->value - eps) return false;
				xi++;
				yi++;
				CHECK_X();
				CHECK_Y();
			} else {
				// x(yind) == 0, y(yind) == yi->value
				if (0 < yi->value - eps) return false;
				yi++;
				CHECK_Y();
			}
		}

main_loop_done:
		if (!xdone) {
			for (; xi != xend; xi++) {
				if (xi->value < -eps) return false;
			}
		} else if (!ydone) {
			for (; yi != yend; yi++) {
				if (0 < yi->value - eps) return false;
			}
		}

		return true;
	}


	/**********************************************************************
	* KMATRIX FUNCTIONS
	**********************************************************************/

	double kmatrix::operator()(unsigned int r, unsigned int c) const
	{
		assert( 0 <= r < size1() );
		assert( 0 <= c < size2() );
		// NOTE: also assumes the kmatrix has been canonicalized

		FOREACH(kmatrix_entry, di,  data) {
			if (di->r == r && di->c == c) {
				return di->value;
			}
		}
		return 0.0;
	}

	void kmatrix::clear(void)
	{
		resize(0,0,0);
	}

	void kmatrix::resize(unsigned int _size1, unsigned int _size2, double value)
	{
		assert( 0 == value );
		size1_ = _size1;
		size2_ = _size2;
		data.clear();
	}

	void kmatrix::push_back(unsigned int r, unsigned int c, double value)
	{
		data.push_back( kmatrix_entry(r,c,value) );
	}

	struct ColumnMajorCompare {
		bool operator()(const kmatrix_entry& lhs, const kmatrix_entry& rhs) {
			return (lhs.c < rhs.c) || ((lhs.c == rhs.c) && (lhs.r < rhs.r));
		}
	};

	bool rc_equal(const kmatrix_entry& lhs, const kmatrix_entry& rhs)
	{
		return (lhs.r == rhs.r) && (lhs.c == rhs.c);
	}

	void kmatrix::canonicalize(void)
	{
		std::vector< kmatrix_entry > d;

		// sort in column-major order
		std::stable_sort( data.begin(), data.end(), ColumnMajorCompare() );

		// ensure there is at most one entry with each (r,c) coordinate.
		// among all the entries with the same (r,c), keep the last one.
		// note that this operation does *not* get rid of near-zero entries.
		FOR ( i, data.size() ) {
			if (!d.empty() && rc_equal( d.back(), data[i] )) {
				d.back() = data[i];
			} else {
				d.push_back( data[i] );
			}
		}
		data.swap( d );
	}

	void kmatrix::read(std::istream& in)
	{
		int rows, cols;
		int num_entries;
		int r, c;
		double value;

		in >> rows >> cols;
		resize( rows, cols );

		in >> num_entries;
		FOR (i, num_entries) {
			in >> r >> c >> value;
			push_back( r, c, value );
		}
	}

	// Index of maximum element of a vector
	int argmax_elt(const DenseVector& v) 
	{
		assert(v.size() > 0);
		double maxval = v(0);
		int max_ind = 0;
		for (unsigned int i=1; i < v.size(); i++) {
			if (v(i) > maxval) {
				max_ind = i;
				maxval = v(i);
			}
		}
		return max_ind;
	}

	int argmax_elt(const SparseVector& v) 
	{
		assert(v.size() > 0);
		double maxval = v(0);
		int max_ind = 0;
		// find the largest non-zero entry
		FOR_CV(v) 
		{
			double val = CV_VAL(v);
			if (val > maxval) 
			{

				max_ind = CV_INDEX(v);
				maxval = val;
			}
		}
		if (maxval >= 0 || v.filled() == v.size()) 
		{
			// a non-zero entry is maximal
			return max_ind;
		}
		else 
		{
			// all non-zero entries are negative; return
			// the index of a zero entry.
			int ind, last_ind = -1;
			FOR_CV(v) 
			{
				ind = CV_INDEX(v);
				if (ind - last_ind > 1) 
				{
					return ind-1;
				}
				last_ind = ind;
			}
			return ind+1;
		}
	}

	void max_assign(DenseVector& result, const DenseVector& x)
	{
		assert( result.size() == x.size() );

		vector<double>::const_iterator  xi = x.data.begin();

		double xval;

		FOREACH_NOCONST(double, ri,  result.data) {
			xval = *xi;
			if (xval > (*ri)) (*ri) = xval;
			xi++;
		}
	}
}


