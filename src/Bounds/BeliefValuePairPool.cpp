#include "BeliefValuePairPool.h"
#include <exception>
using namespace std;


REAL_VALUE BeliefValuePairPool::getValue(SharedPointer<Belief>& b) 
{
	DEBUG_TRACE(cout << "UpperBoundBVpair::getValue" << endl;);

	REAL_VALUE val, min_val;
	REAL_VALUE ratio, min_ratio;
	vector<SparseVector_Entry>::const_iterator  bi, bend;

	vector<SparseVector_Entry>::const_iterator  ci, cend;

	SparseVector tmp, bhat;
	REAL_VALUE cval;
	REAL_VALUE inner_cornerPoints_b;
	REAL_VALUE inner_cornerPoints_c;
	bool bdone = false;

	inner_cornerPoints_b = inner_prod( cornerPoints, *b );

	
	DEBUG_TRACE( cout << "cornerPoints"<< endl; );
	DEBUG_TRACE( cornerPoints.write(cout) << endl; );
	DEBUG_TRACE( cout << "b"<< endl; );
	DEBUG_TRACE( b->write(cout) << endl; );

	DEBUG_TRACE( cout << "inner_cornerPoints_b "<< inner_cornerPoints_b << endl; );

	min_val = inner_cornerPoints_b;

	for(list<SharedPointer<BeliefValuePair> >::iterator iter = points.begin(); iter!=points.end(); iter ++) 
	{

		if ((*iter)->disabled) 
		{
			//cout << "skipping" << endl;
			continue;
		}

		belief_vector& c = *((*iter)->b);
		cval = (*iter)->v;


		//cout << "c.size() " << c.size() << " cval " << cval << endl;

		DEBUG_TRACE( cout << "UpperBound::getValue:1 " << " pr->version: " << (*iter)->version << " cornerPointsVersion " << cornerPointsVersion << endl;);
		if((*iter)->version == cornerPointsVersion) 
		{
			DEBUG_TRACE( cout << "UpperBound::getValue:2 " << endl; );
			inner_cornerPoints_c = (*iter)->innver_cornerPoints;
		}
		else 
		{
			DEBUG_TRACE( cout << "UpperBound::getValue:3 " << endl; );

			inner_cornerPoints_c = inner_prod( cornerPoints, c );
			(*iter)->innver_cornerPoints = inner_cornerPoints_c;
			(*iter)->version = cornerPointsVersion;
		}

		// 			if (inner_cornerPoints_c <= cval)
		// 			{
		// 		  // FIX: should figure out a way to prune c here
		// 		  //cout << "  bad cval" << endl;
		// 				continue;
		// 			}

		min_ratio = 99e+20;

		bi = b->data.begin();
		bend = b->data.end();

		ci = c.data.begin();
		cend = c.data.end();

		for (; ci != cend; ci++) {
			if (0.0 == ci->value) continue;

			// advance until bi->index >= ci->index
			while (1) {
				if (bi == bend) {
					bdone = true;
					goto break_ci_loop;
				}
				if (bi->index >= ci->index) break;
				bi++;
			}

			if (bi->index > ci->index) {
				// we found a j such that b(j) = 0 and c(j) != 0, which implies
				// min_ratio = 0, so this c is useless and should be skipped
				//cout << "  skipping out" << endl;
				goto continue_Points_loop;
			}

			ratio = bi->value / ci->value;
			if (ratio < min_ratio) min_ratio = ratio;
		}
break_ci_loop:
		if (bdone) {
			for (; ci != cend; ci++) {
				if (0.0 != ci->value) goto continue_Points_loop;
			}
		}

		val = inner_cornerPoints_b + min_ratio * ( cval - inner_cornerPoints_c );
#if 1
		if (min_ratio > 1) 
		{
			if (min_ratio < 1 + MIN_RATIO_EPS) 
			{
				// round-off error, correct it down to 1
				min_ratio = 1;
			} 
			else 
			{
				cout << "ERROR: min_ratio > 1 in upperBoundInternal!" << endl;
				cout << "  (min_ratio-1)=" << (min_ratio-1) << endl;
				cout << "  normb=" << b->norm_1() << endl;
				cout << "  b=";
				b->write(cout);
				cout << endl;

				cout << "  normc=" << c.norm_1() << endl;
				cout << "  c=";
				c.write(cout);
				cout << endl;

				exit(EXIT_FAILURE);
			}
		}
#endif

		if (val < min_val) min_val = val;
continue_Points_loop:
		;
	}

	return min_val;
}



REAL_VALUE BeliefValuePairPool::getValue_NoInterpolation(const belief_vector& b) 
{
	REAL_VALUE min_val;
	REAL_VALUE inner_cornerPoints_b;

	inner_cornerPoints_b = inner_prod( cornerPoints, b );
	min_val = inner_cornerPoints_b;

	return min_val;
}

// update helper functions
SharedPointer<BeliefValuePair> BeliefValuePairPool::addPoint(SharedPointer<belief_vector>&  b, REAL_VALUE val) 
{

	DEBUG_TRACE( cout << "BVPairPool::add" << endl; );
	DEBUG_TRACE( cout << "val " << val << endl; );
	DEBUG_TRACE( cout << "b   "; );
	DEBUG_TRACE( b->write(cout) << endl; );

	int wc = whichCornerPoint(b);
	if (-1 == wc) 
	{
		SharedPointer<BeliefValuePair>bvp = new BeliefValuePair();
		// new bvp will have "disabled" set to false
		bvp->b = b;
		bvp->v = val;

		points.push_back(bvp);
		return bvp;
	}
	else 
	{
		cornerPointsVersion++;
		cornerPoints(wc) = val;
		return NULL;
	}
}

// if b is a corner point e_i, return i.  else return -1
int BeliefValuePairPool::whichCornerPoint(const SharedPointer<belief_vector>&  b) const {
	int numStates = problem->getBeliefSize();
	int non_zero_count = 0;
	int non_zero_index = -1;
	FOR(i, numStates) {
		if (fabs((*b)(i)) > CORNER_EPS) {
			if (non_zero_count == 0) {
				non_zero_index = i;
				non_zero_count++;
			} else {
				return -1;
			}
		}
	}
	if (non_zero_count == 0) {
		return -1;
	}
	else {
		return non_zero_index;
	}
}
//private methods
//for debugging purposes
void BeliefValuePairPool::printCorners() const {
	printf("[");

	FOREACH(REAL_VALUE, di,  cornerPoints.data) {
		printf("%f, ", *di);
	}
	printf("]\n");
}//end printAlphal
