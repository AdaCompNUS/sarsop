/** 
 * This code is released under GPL v2 
 * @file BeliefException.h
 * @brief This class extends the Runtime_Exception class and should be used as the base
 * class for throwing exceptions in the project
 * @author Amit Jain
 * @date 2007-03-20
 */

#ifndef BELIEFEXCEPTION_H
#define BELIEFEXCEPTION_H
#include <string>
#include <stdexcept>

namespace pomdp{
	class BeliefException:public std::runtime_error{
		private: 
			/** 
			 * @brief this stores the error message internally
			 */
			std::string err; 
			/** 
			 * @brief to prevent the call of a default constructor
			 */
			BeliefException();
		public: 
			/** 
			 * @brief this is the principal constructor to be called with an error message e
			 * 
			 * @param e error message
			 */
			BeliefException(const std::string& e);
			
			/** 
			 * @brief accessor method for getting the error message
			 * 
			 * @return the error message
			 */
			std::string getMessage();

			/** 
			 * @brief destructor
			 */
			~BeliefException() throw();
	};
}
#endif

