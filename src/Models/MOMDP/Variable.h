#ifndef Variable_H
#define Variable_H


#include <vector>
#include <string>
#include "MOMDP.h"
#include "MObject.h"
#include "IVariableValue.h"
#include "VariableValue.h"
#include "IVariable.h"
using namespace std;
using namespace momdp;

namespace momdp
{
	// generic class for variable, holds user defined variable value
	class Variable : public IVariable
	{
	private:
		string variableName;
		vector<SharedPointer<VariableValue> > values;
		
	public:
		Variable(string name);
		virtual ~Variable(void);
		
		virtual void addValue(SharedPointer<VariableValue> value);
		virtual SharedPointer<IVariableValue> getValueByName(string valName);
		virtual void addValue(string value, double initialProb = 0.0);
		virtual string getVariableName();
		virtual vector<SharedPointer<IVariableValue> > getInitialValues();
		virtual int getNumValues();
		//virtual vector<SharedPointer<VariableValue> > getValues();
		virtual vector<SharedPointer<IVariableValue> > getValues();
	};

}

#endif

