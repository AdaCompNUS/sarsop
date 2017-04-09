#include "VariableCombined.h"

VariableCombined::VariableCombined(string name) : Variable(name)
{

}

VariableCombined::~VariableCombined(void)
{
}

void VariableCombined::addSubVar(SharedPointer<IVariable> subVar)
{
	subVariables.push_back(subVar);
}

SharedPointer<IVariable> VariableCombined::getSubVar(int index)
{
	return subVariables[index];
}

void VariableCombined::seal()
{
	if(subVariables.size() == 0 )
	{
		return;
	}
	vector<vector<SharedPointer<IVariableValue> > > valueListArray;
	vector<int> valueIndex;
	for(int i = 0 ;i < subVariables.size() ; i++)
	{
		SharedPointer<IVariable> subVariable = subVariables[i];
		valueListArray.push_back(subVariable->getValues());
		valueIndex.push_back(0);
	}

	while(valueIndex[0]< valueListArray[0].size())
	{
		string curVal = "";
		for(int i = 0 ; i < valueIndex.size() ; i ++)
		{
			int subIndex = valueIndex[i];
			string curPart = valueListArray[i][subIndex]->getValueName();
			curVal += curPart;
		}

		this->addValue(curVal);

		int lastIndex = valueIndex[(valueIndex.size() - 1)];
		lastIndex ++;
		valueIndex[valueIndex.size() - 1] = lastIndex;

		for(int i = valueIndex.size() -1 ; i > 0  ; i --)
		{
			int subIndex = valueIndex[i];
			if(subIndex >= valueListArray[i].size())
			{
				valueIndex[i]= 0; // carry in
				int subIndex2 = valueIndex[i - 1];
				subIndex2 ++;
				valueIndex[i - 1] = subIndex2;
			}
		}
	}



}

