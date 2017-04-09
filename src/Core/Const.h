// Solution Global Definition
#ifndef Const_H
#define Const_H

typedef double REAL_VALUE;
#define BELIEF_DECIMAL_PRECISION 9

#undef FOREACH_NOCONST
#define FOREACH_NOCONST(type,elt,collection) \
	for (std::vector<type>::iterator elt=(collection).begin(), __end=(collection).end(); elt != __end;	elt++)

#undef LISTFOREACH_NOCONST
#define LISTFOREACH_NOCONST(type,elt,collection) \
	for (std::list<type>::iterator elt=(collection).begin(), __end=(collection).end(); elt != __end;	elt++)

#undef FOREACH
#define FOREACH(type,elt,collection) \
	for (std::vector<type>::const_iterator elt=(collection).begin(), __end=(collection).end(); elt != __end;	elt++)

#undef LISTFOREACH
#define LISTFOREACH(type,elt,collection) \
	for (std::list<type>::const_iterator elt=(collection).begin(), __end=(collection).end(); elt != __end;	elt++)


#undef FOR
#define FOR(i,n) \
	for (unsigned int i=0, __n = (n); i<__n; i++)

#undef FOR_CV
#define FOR_CV(v) \
	for ( std::vector<SparseVector_Entry>::const_iterator  __cv_i=v.data.begin(); __cv_i != v.data.end(); __cv_i++)

#define CV_VAL(v)   (__cv_i->value)
#define CV_INDEX(v) (__cv_i->index)

#define SPARSE_EPS (1e-10)
#define OBS_IS_ZERO_EPS (1e-10)
#define ZMDP_BOUNDS_PRUNE_EPS (1e-10)

#define state_vector SparseVector
#define belief_vector SparseVector


//#define outcome_prob_vector DenseVector
//#define obs_prob_vector DenseVector
//#define obsState_prob_vector DenseVector

#define outcome_prob_vector SparseVector
#define obs_prob_vector SparseVector
#define obsState_prob_vector SparseVector

#define alpha_vector DenseVector


typedef int state_val;


#ifdef DEBUG_TRACE_ON
	#define DEBUG_TRACE(v) v
#else
	#define DEBUG_TRACE(v)
#endif


#ifdef DEBUG_LOG_ON
	#define DEBUG_LOG(v) v
#else
	#define DEBUG_LOG(v)
#endif


#ifdef DEBUG_TRACE_ON1
	#define DEBUG_TRACE1(v) v
#else
	#define DEBUG_TRACE1(v)
#endif


#endif

