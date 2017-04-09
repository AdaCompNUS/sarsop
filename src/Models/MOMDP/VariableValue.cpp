#include "VariableValue.h"

VariableValue::VariableValue(string varName, string valueName, int index, double prob)
{
	this->varName = varName;
	this->valueName = valueName;
	this->prob = prob;
	this->index = index;
}
string VariableValue::getVariableName()
{
	return varName;
}
double VariableValue::getProb()
{
	return prob;
}

string VariableValue::getValueName()
{
	return valueName;
}

string VariableValue::ToString()
{
	return valueName;
}

VariableValue::~VariableValue(void)
{
}

int VariableValue::getIndex()
{
	return index;
}

bool VariableValue::equals(SharedPointer<IVariableValue> obj)
{
	//SharedPointer<VariableValue> ptr = dynamic_pointer_cast<VariableValue>(obj);
	//return valueName.compare(ptr->valueName) == 0;
	assert (this->getVariableName() == obj->getVariableName());
	return getIndex() == obj->getIndex();
}