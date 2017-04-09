#ifndef ParserSelector_H
#define ParserSelector_H
#include <fstream>
#include "MOMDP.h"
#include "FactoredPomdp.h"
#include "solverUtils.h"

using namespace momdp;
using namespace std;
namespace momdp 
{

	class ParserSelector
	{
	public:
		ParserSelector(void);
		~ParserSelector(void);
		static SharedPointer<MOMDP> loadProblem(string problemName, SolverParams& solverParam);
	};
}
#endif

