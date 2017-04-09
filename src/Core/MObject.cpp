#include "MObject.h"
#include "GlobalResource.h"
#include <cstdlib>
#include <sstream>
#include <new>

using namespace std;
using namespace momdp;
namespace momdp 
{
	MObject::MObject(void) : referenceCount(0)
	{
		
	}

	MObject::~MObject(void)
	{
	}
	void *MObject::operator new(size_t nSize)
	{
		void *p;

		//cout << "new devector.\n";

		p =  malloc(nSize);
		//p = (void *) new char [nSize];

		GlobalResource::getInstance()->memoryUsage += nSize;
		GlobalResource::getInstance()->checkMemoryUsage();

		if(!p) 
		{
			bad_alloc ba;
			throw ba;
			//throw new bad_alloc();
		}
		MObject *objP = (MObject *)p;

		objP->thisSize = nSize;

		return p;
	}

	// delete operator overloaded
	void MObject::operator delete(void *p)
	{
		MObject *objP = (MObject *)p;
		//cout << "delete DenseVector.\n";
		GlobalResource::getInstance()->memoryUsage -= objP->thisSize;
		free(p);
		//delete [] p;
	}

	// ToString
	string MObject::ToString()
	{
		stringstream sb;
		sb << "Object at " << this;

		return sb.str();
	}

}