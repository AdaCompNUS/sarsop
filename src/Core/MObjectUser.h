#ifndef MObjectUser_H
#define MObjectUser_H

#include "Const.h"
//using namespace momdp;
namespace momdp 
{

class MObject;

template <typename T>
class MObjectUser
{
public:
	MObjectUser(void)
	{
	}
	virtual void forcedDelete(T* pointer) = 0;
	virtual ~MObjectUser(void)
	{
	}
};

}
#endif



