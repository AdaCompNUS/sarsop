/** 
 * This code is released under GPL v2
 */
#include "BeliefException.h"

namespace pomdp{

BeliefException::BeliefException(const std::string& e):runtime_error(e){
	err = e;
}

std::string BeliefException::getMessage(){
	return err;
}

BeliefException::~BeliefException() throw(){ 
}
}
