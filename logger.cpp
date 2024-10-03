/*********************************************************************
 * @file logger.cpp
 * 
 * @brief Source file for logging.
 *********************************************************************/

#include "logger.hpp"

using namespace std;

namespace logger {

void ERROR_VariableNotDeclared(SA *analyzer, node::Node *sym) {
    cerr << "ERROR(" << sym->getLine() << "): Symbol '" << sym->getString() << "' is not declared." << endl;
    analyzer->incErrors();
}

void ERROR_VariableAlreadyDeclared(SA *analyzer, node::Node *sym, node::Node *decl) {
    cerr << "ERROR(" << sym->getLine() << "): Symbol '" << sym->getString() << "' is already declared at line ";
    cerr << decl->getLine() << "." << endl;
    analyzer->incErrors();
}

}