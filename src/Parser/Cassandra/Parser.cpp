/************************************************************
*File: 	Parser.cc
*Date: 	19/03/2006 
*Brief: for parsing pomdp files into pomdp objects and 
*	converting resulting alpha plane structures into 
*	policy files
*************************************************************/

#include "Parser.h"

using namespace std;
using namespace momdp;

namespace momdp{
  //constructor
  Parser::Parser(void)
  {
  }

  //methods
  POMDP* Parser::parse(string probName, bool useFastParser)
  {
    return new POMDP(probName, useFastParser);
  }//end parse
}; 
