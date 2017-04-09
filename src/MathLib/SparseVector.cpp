#include "SparseVector.h"

#include <iostream>
#include <cassert>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <stdexcept>
#include <limits>

#include "MathLib.h"
#include "md5.h"
using namespace std;

using namespace momdp;
namespace momdp
{
    void SparseVector::add(SparseVector& result,	const SparseVector& x, 	const SparseVector& y)
    {
	vector<SparseVector_Entry>::const_iterator  xi, xend;

	vector<SparseVector_Entry>::const_iterator  yi, yend;

	unsigned int xind, yind;
	bool xdone = false, ydone = false;

	assert( x.size() == y.size() );
	result.resize( x.size() );

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
		result.push_back( xind, xi->value );
		xi++;
		CHECK_X();
	    } else if (xind == yind) {
		result.push_back( xind, xi->value + yi->value );
		xi++;
		yi++;
		CHECK_X();
		CHECK_Y();
	    } else {
		result.push_back( yind, yi->value );
		yi++;
		CHECK_Y();
	    }
	}

main_loop_done:
	if (!xdone) {
	    for (; xi != xend; xi++) {
		result.push_back( xi->index, xi->value );
	    }
	} else if (!ydone) {
	    for (; yi != yend; yi++) {
		result.push_back( yi->index, yi->value );
	    }
	}

	//result.finalize();
    }

    REAL_VALUE SparseVector::getEntropy()
    {
	REAL_VALUE entropy = 0;
	FOREACH (SparseVector_Entry, di, data) 
	{
	    entropy += di->value * ( log(di->value) / log(2.0) );
	}
	entropy = (-1) * entropy;//negate summation
	return entropy;
    }

    void SparseVector::subtract(SparseVector& result,	const SparseVector& x, const SparseVector& y)
    {
	vector<SparseVector_Entry>::const_iterator  xi, xend;

	vector<SparseVector_Entry>::const_iterator  yi, yend;

	unsigned int xind, yind;
	bool xdone = false, ydone = false;

	assert( x.size() == y.size() );
	result.resize( x.size() );

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
		result.push_back( xind, xi->value );
		xi++;
		CHECK_X();
	    } else if (xind == yind) {
		result.push_back( xind, xi->value - yi->value );
		xi++;
		yi++;
		CHECK_X();
		CHECK_Y();
	    } else {
		result.push_back( yind, -yi->value );
		yi++;
		CHECK_Y();
	    }
	}

main_loop_done:
	if (!xdone) {
	    for (; xi != xend; xi++) {
		result.push_back( xi->index, xi->value );
	    }
	} else if (!ydone) {
	    for (; yi != yend; yi++) {
		result.push_back( yi->index, -yi->value );
	    }
	}

	//result.finalize();
    }


    SparseVector::SparseVector(void) : logicalSize(0)
    {

    }

    SparseVector::~SparseVector(void)
    {
    }


    // Arithmetic
    int SparseVector::argSampleDist() const
    {
	REAL_VALUE randNumber = unit_rand();
	REAL_VALUE sum = 0.0;
	FOREACH(SparseVector_Entry, entry,  data) 
	{
	    sum += entry->value;
	    if(randNumber < sum )
	    {
		return entry->index; 
	    }
	}
	return size()-1;
    }

    bool SparseVector::isDifferentByAtLeastSingleEntry(const SparseVector& x, const REAL_VALUE& threshold) const
    {
	std::vector<SparseVector_Entry>::const_iterator otherIter = x.data.begin();
	std::vector<SparseVector_Entry>::const_iterator iter = data.begin();
	for(; iter != data.end(); iter++)
	{
	    while( (otherIter != x.data.end()) && (iter->index > otherIter->index))
	    {
		if(fabs(otherIter->value) > threshold)
		{
		    return true;
		}
		otherIter++;
	    }

	    if((otherIter != x.data.end()) && (iter->index == otherIter->index))
	    {
		if(fabs(iter->value - otherIter->value) > threshold)
		{
		    return true;
		}
		otherIter++;
	    }
	    else
	    {
		if( fabs(iter->value) > threshold)
		{
		    return true;
		}
	    }

	}
	return false;
    }

    //return the index of maximum SparseEntry
    //in case of tie, only the index of first max is returned
    int SparseVector::argmax()
    {
	REAL_VALUE maxValue = -1 * numeric_limits<REAL_VALUE>::max();;
	int maxIndex = -1;
	FOREACH(SparseVector_Entry, di,  data) {
	    if (di->value > maxValue) {
		maxIndex = di->index;
		maxValue = di->value;
	    }
	}
	return maxIndex;
    }


    REAL_VALUE SparseVector::operator()( int index) const
    {
	FOREACH(SparseVector_Entry, di,  data) {
	    if (di->index >= index) {
		if (di->index == index) {
		    return di->value;
		} else {
		    return 0.0;
		}
	    }
	}
	return 0.0;
    }

    void SparseVector::operator*=(REAL_VALUE s)
    {
	vector<SparseVector_Entry>::iterator  vi, vend = data.end();

	for (vi = data.begin(); vi != vend; vi++) {
	    vi->value *= s;
	}
    }

    REAL_VALUE SparseVector::maskedSum( vector<int> mask)
    {
	REAL_VALUE result = 0.0;
	vector<SparseVector_Entry>::iterator  vi, vend = data.end();

	for (vi = data.begin(); vi != vend; vi++) 
	{
	    if (!mask[vi->index]) 
	    {
		result += vi->value;
	    }
	}
	return result;
    }

    void SparseVector::copyIndex( vector<int>& result)
    {
	vector<SparseVector_Entry>::iterator  vi, vend = data.end();
	for (vi = data.begin(); vi != vend; vi++) 
	{
	    result.push_back(vi->index);
	}
    }

    void SparseVector::copyValue( vector<REAL_VALUE>& result)
    {
	vector<SparseVector_Entry>::iterator  vi, vend = data.end();
	for (vi = data.begin(); vi != vend; vi++) 
	{
	    result.push_back(vi->value);
	}
    }

    // TODO:: this implementation may leak memory... check

    void SparseVector::operator+=(const SparseVector& x)
    {
	SparseVector tmp;
	add(tmp,*this,x);
	*this = tmp;
    }

    void SparseVector::operator-=(const SparseVector& x)
    {
	SparseVector tmp;
	subtract(tmp,*this,x);
	*this = tmp;
    }

    bool SparseVector::operator==(const SparseVector& x) const
    {
	if(x.logicalSize != logicalSize){
	    return false;
	}
	std::vector<SparseVector_Entry>::const_iterator otherIter = x.data.begin();
	std::vector<SparseVector_Entry>::const_iterator iter = data.begin();
	for(; iter != data.end();
		otherIter++, iter++){
	    if(iter->index != otherIter -> index){
		return false;
	    }
	    if(iter->value != otherIter -> value){
		return false;
	    }
	}
	return true;
    }

    REAL_VALUE SparseVector::delta(const SparseVector& x) const{
	REAL_VALUE del = 0;
	if(x.data.size() != data.size()){
	    return 200000;
	}
	std::vector<SparseVector_Entry>::const_iterator otherIter = x.data.begin();
	std::vector<SparseVector_Entry>::const_iterator iter = data.begin();
	for(; iter != data.end(); iter++, otherIter++){
	    if(iter->index != otherIter->index){
		return 1000000;
	    }

	    if(fabs(iter->value - otherIter-> value) > del){
		del = fabs(iter->value - otherIter->value);
	    }
	}
	return del;
    }


    REAL_VALUE SparseVector::totalDifference(const SparseVector& x) const
    {
	REAL_VALUE del = 0;

	std::vector<SparseVector_Entry>::const_iterator otherIter = x.data.begin();
	std::vector<SparseVector_Entry>::const_iterator iter = data.begin();
	for(; iter != data.end(); iter++)
	{
	    while( (otherIter != x.data.end()) && (iter->index > otherIter->index))
	    {
		del += fabs(otherIter->value);
		otherIter++;
	    }

	    if((otherIter != x.data.end()) && (iter->index == otherIter->index))
	    {
		del += fabs(iter->value - otherIter->value);
		otherIter++;
	    }
	    else
	    {
		del += fabs(iter->value);
	    }

	}
	return del;
    }

    int SparseVector::sampleState() const
    {
	REAL_VALUE randNumber = unit_rand();
	REAL_VALUE sum = 0.0;
	FOREACH(SparseVector_Entry, entry,  data) 
	{
	    sum += entry->value;
	    if(randNumber < sum )
	    {
		return entry->index; 
	    }
	}
	return logicalSize -1;
    }

    REAL_VALUE SparseVector::totalHashDifference(const SparseVector& x) const
    {
	REAL_VALUE del = 0;

	std::vector<SparseVector_Entry>::const_iterator otherIter = x.data.begin();
	std::vector<SparseVector_Entry>::const_iterator iter = data.begin();
	for(; iter != data.end(); iter++)
	{
	    while( (otherIter != x.data.end()) && (iter->index > otherIter->index))
	    {
		REAL_VALUE toTruncate = otherIter->value;
		toTruncate *=2^20; // TODO:: this truncation is different from the BeliefEntryTruncate function used in the hash generation function, fix it?
		toTruncate = floor(toTruncate);

		del += fabs(toTruncate);
		otherIter++;
	    }

	    if((otherIter != x.data.end()) && (iter->index == otherIter->index))
	    {
		REAL_VALUE toTruncate = iter->value;
		toTruncate *=2^20;
		toTruncate = floor(toTruncate);

		REAL_VALUE toTruncate2 = otherIter->value;
		toTruncate2 *=2^20;
		toTruncate2 = floor(toTruncate);

		del += fabs(toTruncate - toTruncate2);
		otherIter++;
	    }
	    else
	    {
		REAL_VALUE toTruncate = iter->value;
		toTruncate *=2^20;
		toTruncate = floor(toTruncate);


		del += fabs(toTruncate);
	    }

	}
	return del;
    }

    void SparseVector::resize( int _size)
    {
	logicalSize = _size;
	data.clear();
    }

    void SparseVector::push_back( int index, REAL_VALUE value)
    {
	data.push_back( SparseVector_Entry( index, value ) );
    }

    void SparseVector::read(std::istream& in)
    {
	int num_entries;

	in >> logicalSize;
	in >> num_entries;
	data.resize( num_entries );
	FOR (i, num_entries) {
	    in >> data[i].index >> data[i].value;
	}
    }

    //@added by amit on 29th june
    std::ostream& SparseVector::write(std::ostream& out) const{
	out << "size: "<< logicalSize <<",\n data: [";

	for(std::vector<SparseVector_Entry>::const_iterator iter = data.begin();
		iter != data.end(); iter++){
	    out << iter->index << "= "<< iter->value;
	    if(iter < data.end()-1){
		out <<", ";
	    }
	    else{
		out << "]";
	    }
	}
	return out;
    }

    string SparseVector::ToString() const
    {
	stringstream out;
	out << "size: "<< logicalSize <<",\n data: [";
	printf("this: %X\n", this);
	cout << "size: "<< logicalSize <<",\n data: [";

	for(std::vector<SparseVector_Entry>::const_iterator iter = data.begin();	iter != data.end(); iter++)
	{
	    printf("Iter: %X\n", iter);
	    out << iter->index << "= "<< iter->value;
	    if(iter < data.end()-1){
		out <<", ";
	    }
	    else{
		out << "]";
	    }
	}

	out.flush();
	return out.str();
    }


    void SparseVector::finalize() 
    {
	// become immutable
	// TODO:: Temp Disabled

	md5hash = getHashFromSparseVectorTruncated(*this);
	//md5hash = getHashFromCVector(*this)	;
    }

    string SparseVector::md5HashValue()
    {
	if(md5hash.size() ==0)
	{
	    throw runtime_error("Bug, belief sparse vector need to call finalize() method to compute hash value");
	}

	// Sanity check
	//string str1 = getHashFromCVector(*this);
	//if(str1.compare(md5hash) != 0)
	//{
	//	throw runtime_error("Bug, cached md5 is different now, someone changed the sparse vector after finalized method....");
	//}

	return md5hash;
    }

    string SparseVector::convToString(unsigned char *bytes)
    {
	stringstream out;

	// TODO, use setf 
	//out.setf ( ios::hex, ios::basefield );       // set hex as the basefield
	//out.width(2);

	for(int i=0; i<16; i++)
	{
	    out << setfill('0') << setw(2) << hex << (int) bytes[i];
	}	

	return out.str();
    }

    string SparseVector::getHashFromCVector(SparseVector& x)	
    {
	MD5_CTX context;

	unsigned char buffer[1024], digest[16];

	//init md5
	MD5 md5;
	md5.MD5Init (&context);

	std::vector<SparseVector_Entry>::const_iterator iter = x.data.begin();


	for(; iter != x.data.end();iter++)
	{
	    int len1 = sizeof (iter->index);
	    md5.MD5Update (&context, (unsigned char *)&(iter->index), len1);

	    double toTruncate = iter->value;
	    int len2 = sizeof (toTruncate);
	    toTruncate *=2^20;
	    toTruncate = floor(toTruncate);
	    md5.MD5Update (&context, (unsigned char *)&(toTruncate), len2);

	}
	/*
	   generate hash, close the file and return the
	   hash as std::string
	   */
	md5.MD5Final (digest, &context);
	string result = convToString(digest);
	return result;
    }	
    string SparseVector::getHashFromSparseVectorTruncated(SparseVector& x)	
    {
	MD5_CTX context;

	unsigned char digest[16];

	//init md5
	MD5 md5;
	md5.MD5Init (&context);

	std::vector<SparseVector_Entry>::const_iterator iter = x.data.begin();


	for(; iter != x.data.end();iter++)
	{
	    int len1 = sizeof (iter->index);
	    md5.MD5Update (&context, (unsigned char *)&(iter->index), len1);

	    REAL_VALUE toTruncate = BeliefEntryTruncate(iter->value);
	    int len2 = sizeof (toTruncate);

	    md5.MD5Update (&context, (unsigned char *)&(toTruncate), len2);

	}
	/*
	   generate hash, close the file and return the
	   hash as std::string
	   */
	md5.MD5Final (digest, &context);
	string result = convToString(digest);
	return result;

    }	

}

