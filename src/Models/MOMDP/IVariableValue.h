#ifndef IVariableValue_H
#define IVariableValue_H


#include <vector>
#include <string>
#include "MObject.h"
using namespace std;
using namespace momdp;
namespace momdp
{
	// pure interface for Variable Value
	class IVariableValue : public MObject
	{
	public:
		//virtual string ToString() = 0;
		virtual string getVariableName() = 0;
		virtual string getValueName() = 0;
		virtual double getProb() = 0;
		virtual int getIndex() = 0;
		virtual bool equals(SharedPointer<IVariableValue> obj) = 0;

		IVariableValue(void);
		virtual ~IVariableValue(void);
	};
}

#endif

