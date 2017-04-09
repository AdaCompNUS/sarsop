#ifndef VariableValue_H
#define VariableValue_H


#include <vector>
#include <string>
#include "IVariableValue.h"
#include "MObject.h"
using namespace std;
using namespace momdp;

namespace momdp
{
	// Generic variable value
	class VariableValue : public IVariableValue
	{
	private:
		double prob;
		string valueName;
		string varName;
		int index;

	public:
		virtual string ToString();
		virtual double getProb();
		virtual string getVariableName();
		virtual string getValueName();
		virtual int getIndex();
		virtual bool equals(SharedPointer<IVariableValue> obj);

		VariableValue(string varName, string valueName, int index, double prob = 0.0);
		virtual ~VariableValue(void);
	};
}

#endif

