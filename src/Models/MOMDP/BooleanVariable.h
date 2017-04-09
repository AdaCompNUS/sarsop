#ifndef BooleanVariable_H
#define BooleanVariable_H


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


	// Boolean Variable
	class BooleanVariable : public IVariable
	{
	private:
		string variableName;
		vector<SharedPointer<VariableValue> > values;
		virtual void addValue(SharedPointer<VariableValue> value);
		virtual void addValue(string value, double initialProb = 0.0);

	public:
		static const int TrueValueIndex = 0;
		static const int FalseValueIndex = 1;
		BooleanVariable(string name,double initProbTrue, double initProbFalse);
		virtual ~BooleanVariable(void);
		virtual SharedPointer<IVariableValue> getValueByName(string valName);
		virtual SharedPointer<IVariableValue> getTrueValue();
		virtual SharedPointer<IVariableValue> getFalseValue();
		virtual string getVariableName();
		virtual vector<SharedPointer<IVariableValue> > getInitialValues();
		virtual int getNumValues();
		virtual vector<SharedPointer<IVariableValue> > getValues();
	};

}

#endif

