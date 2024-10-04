/*********************************************************************
 * @file logger.cpp
 * 
 * @brief Source file for logging.
 *********************************************************************/

#include "logger.hpp"

using namespace std;

namespace logger {

/***********************************************
*  ERRORS
***********************************************/

void ERROR_VariableNotDeclared(SA *analyzer, node::Node *sym) {
    cerr << "ERROR(" << sym->getLine() << "): Symbol '" << sym->getString() << "' is not declared." << endl;
    analyzer->incErrors();
}

void ERROR_VariableAlreadyDeclared(SA *analyzer, node::Node *sym, node::Node *decl) {
    cerr << "ERROR(" << sym->getLine() << "): Symbol '" << sym->getString() << "' is already declared at line ";
    cerr << decl->getLine() << "." << endl;
    analyzer->incErrors();
}

void ERROR_VariableAsFunction(SA *analyzer, node::Node *sym) {
    cerr << "ERROR(" << sym->getLine() << "): Cannot use function '" << sym->getString()  <<"' as a variable." << endl;
    analyzer->incErrors();
}

void ERROR_Linker(SA *analyzer) {
    cerr << "ERROR(LINKER): A function named 'main()' must be defined." << endl;
    analyzer->incErrors();
}

/***********************************************
*  WARNINGS
***********************************************/

void WARN_VariableNotUsed(SA *analyzer, node::Node *sym) {
    cerr << "WARNING(" << sym->getLine() << "): The variable '" << sym->getString() << "' seems not to be used." << endl;
    analyzer->incWarnings();
}

}