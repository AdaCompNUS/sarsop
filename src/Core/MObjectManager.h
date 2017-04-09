#ifndef MObjectManager_H
#define MObjectManager_H

#include <list>
using namespace std;

#include "MObjectUser.h"


using namespace momdp;
namespace momdp 
{

class MObject;

template <typename T>
class MObjectManager
{
private:
	list<MObjectUser<T> *> userList;

public:

	MObjectManager(void)
	{
	}
	virtual ~MObjectManager(void)
	{
	}

	// delete the MObject from everywhere
	virtual void strongDelete(T *pointer)
	{
		for(typename list<MObjectUser<T> *>::iterator iter = userList.begin(); iter != userList.end(); iter ++)
		{
			(*iter)->forcedDelete(pointer);
		}
		localDelete(pointer);
	}


	// one user deleted the MObject, but the MObject may still be used by other users. Only decrease refCount
	virtual void weakDelete(T *pointer)
	{
		// TODO: obsolete this by using smart pointers
		//pointer->refCount -- ;
		//if(pointer->refCount == 1)
		//{
		//	// only the manager is using it, may do something here...
		//}
		//else if(pointer->refCount == 0)
		//{
		//	// no one is using it, delete
		//	localDelete(pointer);
		//}


	}
	// this function perform the actual delete, must be instantiated
	virtual void localDelete(T *pointer) = 0; 


	virtual void registerUser(MObjectUser<T> *pointer)
	{
		userList.push_back(pointer);
	}
	virtual void removeUser(MObjectUser<T> *pointer)
	{
		userList.remove(pointer);
	}

	
};
}

#endif

