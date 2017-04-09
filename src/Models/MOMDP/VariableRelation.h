#ifndef VariableRelation_H
#define VariableRelation_H


#include <vector>
#include <map>
#include <string>
#include "MObject.h"
#include "IVariableValue.h"
#include "VariableValue.h"
#include "IVariable.h"
using namespace std;
using namespace momdp;

namespace momdp
{
	// relation entry
	class RelEntry : public MObject
	{
	public:
		//map<string, SharedPointer<IVariableValue> > sourceValues;
		map<string, SharedPointer<IVariableValue> > destValues;
		double prob;
	};
	// generic class for variable, holds user defined variable value
	class VariableRelation : public MObject
	{
	protected:
		vector<SharedPointer<IVariable> > srcVars;
		SharedPointer<IVariable> destVar;
		
	public:
		VariableRelation();
		virtual ~VariableRelation(void);
		
		virtual void addSourceVar(SharedPointer<IVariable> var);
		virtual void setDestVariable(SharedPointer<IVariable> var);
		virtual vector<SharedPointer<IVariable> > getSourceVars();
		virtual SharedPointer<IVariable> getDestVariable();
		
		virtual vector<SharedPointer<RelEntry> > getProb(map<string, SharedPointer<IVariableValue> > sourceVals) = 0;
		
	};

	

}

#endif

