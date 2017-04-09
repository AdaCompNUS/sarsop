#include "VariableRelation.h"

VariableRelation::VariableRelation()
{
}

VariableRelation::~VariableRelation(void)
{
}
void VariableRelation::addSourceVar(SharedPointer<IVariable> var)
{
	srcVars.push_back(var);
}
void VariableRelation::setDestVariable(SharedPointer<IVariable> var)
{
	destVar = var;
}
vector<SharedPointer<IVariable> > VariableRelation::getSourceVars()
{
	return srcVars;
}
SharedPointer<IVariable> VariableRelation::getDestVariable()
{
	return destVar;
}