
#include "Symbol.h"
#include "utility.h"

int ID_of_Symbols = 0;
void Symbol::Symbol::RESET() { ID_of_Symbols = 0; }


Symbol::~Symbol() {
	delete_verbose("@Memory: Symbol deleted (%s)\n", this->toString().c_str());
}

Symbol::Symbol(std::string name) : my_id(ID_of_Symbols++), name(name) {}


Symbol::Symbol(Symbol* symbol) : my_id(symbol->my_id), name(symbol->name) {}


std::string Symbol::getName() const { return this->name; }


const int Symbol::getId() const { return this->my_id; }


std::string Symbol::Symbol::toString (Symbol* symbol) { return symbol->toString(); }


std::string Symbol::toString() const { return this->name; }


/*
int Symbol::cmp (const Symbol* other) const { return this->id - other->id; }
bool Symbol::operator< (const Symbol* other) const { return this->cmp(other) < 0;}
bool Symbol::operator<= (const Symbol* other) const { return this->cmp(other) <= 0;}
bool Symbol::operator> (const Symbol* other) const { return this->cmp(other) > 0;}
bool Symbol::operator>= (const Symbol* other) const { return this->cmp(other) >= 0;}
bool Symbol::operator== (const Symbol* other) const { return this->cmp(other) == 0; }
bool Symbol::operator!= (const Symbol* other) const { return this->cmp(other) != 0; }
*/

