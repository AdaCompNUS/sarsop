#include "Variable.h"
#include <exception>


Variable::Variable(string name)
{
	this->variableName = name;
}

Variable::~Variable(void)
{
}
vector<SharedPointer<IVariableValue> > Variable::getValues()
{
	vector<SharedPointer<IVariableValue> > result;
	for(int i = 0; i < values.size(); i++)
	{
		result.push_back(values[i]);
	}
	return result;
}

void  Variable::addValue(SharedPointer<VariableValue> value)
{
	values.push_back(value);
}
void  Variable::addValue(string value, double initialProb)
{
	SharedPointer<VariableValue> newValue (new VariableValue(this->getVariableName(), value, values.size(), initialProb));
	addValue(newValue);
}
int Variable::getNumValues()
{
	return values.size();
}
string Variable::getVariableName()
{
	return variableName;
}

vector<SharedPointer<IVariableValue> > Variable::getInitialValues()
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

SharedPointer<IVariableValue> Variable::getValueByName(string valName)
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

