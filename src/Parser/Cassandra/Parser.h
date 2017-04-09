/** 
 * @file Parser.h
 * @brief Defines the parser class for POMDP object parsing
 * @author rong nan
 * @date 2007-03-18
 */

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include "POMDP.h"

using namespace std;

namespace momdp{

class Parser{
  public:
    /** 
     * @brief default constructor
     */
    Parser();

    /** 
     * @brief parses a file into POMDP object
     * 
     * @param file: the file to be parsed
     * 
     * @return POMDP object as the result of parsing
     */
    POMDP* parse(string probName, bool useFastParser);
    
    /** 
     * @brief default destructor
     */
    ~Parser();

};

}
#endif
