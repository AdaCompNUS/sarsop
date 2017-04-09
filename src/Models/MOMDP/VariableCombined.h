#ifndef VariableCombined_H
#define VariableCombined_H


#include <vector>
#include <string>
#include "MOMDP.h"
#include "MObject.h"
#include "IVariableValue.h"
#include "VariableValue.h"
#include "IVariable.h"
#include "Variable.h"
using namespace std;
using namespace momdp;

namespace momdp
{
	// generic class for variable, holds user defined variable value
	class VariableCombined : public Variable
	{
	private:
		//string variableName;
		//vector<SharedPointer<VariableValue> > values;
		vector<SharedPointer<IVariable> > subVariables;

	public:
		VariableCombined(string name);
		virtual ~VariableCombined(void);


		void addSubVar(SharedPointer<IVariable> subVar);
		SharedPointer<IVariable> getSubVar(int index);
		void seal();



	};

}

#endif

