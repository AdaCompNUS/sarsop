#include "BooleanVariable.h"
#include "VariableValue.h"
#include <exception>


BooleanVariable::BooleanVariable(string name, double initProbTrue, double initProbFalse)
{
	this->variableName = name;
	addValue("true", initProbTrue);
	addValue("false", initProbFalse);
}

BooleanVariable::~BooleanVariable(void)
{
}
vector<SharedPointer<IVariableValue> > BooleanVariable::getValues()
{
	vector<SharedPointer<IVariableValue> > result;
	for(int i = 0; i < values.size(); i++)
	{
		result.push_back(values[i]);
	}
	return result;
}

void  BooleanVariable::addValue(SharedPointer<VariableValue> value)
{
	values.push_back(value);
}
void  BooleanVariable::addValue(string value, double initialProb)
{
	SharedPointer<VariableValue> newValue (new VariableValue(this->getVariableName(), value, values.size(), initialProb));
	addValue(newValue);
}
int BooleanVariable::getNumValues()
{
	return values.size();
}
string BooleanVariable::getVariableName()
{
	return variableName;
}

vector<SharedPointer<IVariableValue> > BooleanVariable::getInitialValues()
{
	vector<SharedPointer<IVariableValue> > result;
	for(int i = 0; i < values.size(); i++)
	{
		if(values[i]->getProb() > 0.0001)
		{
			result.push_back(values[i]);
		}
	}
	return result;
}

SharedPointer<IVariableValue> BooleanVariable::getTrueValue()
{
	return values[0];
}

SharedPointer<IVariableValue> BooleanVariable::getFalseValue()
{
	return values[1];
}
SharedPointer<IVariableValue> BooleanVariable::getValueByName(string valName)
{
	for(int i = 0; i < values.size(); i++)
	{
		if(values[i]->getValueName().compare(valName) == 0)
		{
			return values[i];
		}
	}

	throw runtime_error("Cannot find value : " + valName + " in variable : " + this->getVariableName());
}

