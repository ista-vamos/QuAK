#include "Symbol.h"
#include "utility.h"

int ID_of_Symbols = 0;


Symbol::Symbol(std::string name) : id(ID_of_Symbols++), name(name) {}


Symbol::~Symbol() { delete_verbose("@Memory: Symbol deleted\n"); }


std::string Symbol::getName() const { return this->name; }


const int Symbol::getId() const { return this->id; }


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

