
#include "Symbol.h"
#include "utility.h"

unsigned int ID_of_Symbols = 0;
void Symbol::Symbol::RESET() { ID_of_Symbols = 0; }
void Symbol::Symbol::RESET(unsigned int n) { ID_of_Symbols = n; }


Symbol::~Symbol() {
	delete_verbose("@Memory: Symbol deleted (%s)\n", this->toString().c_str());
}

Symbol::Symbol(std::string name) : my_id(ID_of_Symbols++), name(name) {}


Symbol::Symbol(Symbol* symbol) : my_id(symbol->my_id), name(symbol->name) {}


std::string Symbol::getName() const { return this->name; }


const unsigned int Symbol::getId() const { return this->my_id; }


std::string Symbol::Symbol::toString (Symbol* symbol) { return symbol->toString(); }


std::string Symbol::toString() const { return this->name; }


