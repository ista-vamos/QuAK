
#include "Edge.h"
#include "utility.h"


Edge::Edge(Symbol* symbol, Weight<weight_t>* weight, State* from, State* to) :
	symbol(symbol), weight(weight), from(from), to(to) {}


Edge::~Edge () {
	delete_verbose("@Memory: Edge deleted (%s)\n", this->toString().c_str());
}


Symbol* Edge::getSymbol() const {
	return this->symbol;
}


Weight<weight_t>* Edge::getWeight() const {
	return this->weight;
}


State* Edge::getFrom() const {
	return this->from;
}


State* Edge::getTo() const {
	return this->to;
}


std::string Edge::Edge::toString(Edge* edge) {
	return edge->toString();
}


std::string Edge::toString() const {
	std::string s = "";
	s.append(this->symbol->toString());
	s.append(" : ");
	s.append(this->weight->toString());
	s.append(", ");
	s.append(this->from->getName());
	s.append(" -> ");
	s.append(this->to->getName());
	return s;
}

/*
int Edge::cmp (const Edge* other) const {
	int value = this->weight - other->weight;
	value += value==0 ? this->from->cmp(other->from) : 0;
	value += value==0 ? this->to->cmp(other->to) : 0;
	value += value==0 ? this->symbol->cmp(other->symbol) : 0;
	return value;
}
bool Edge::operator< (const Edge* other) const { return this->cmp(other) < 0;}
bool Edge::operator<= (const Edge* other) const { return this->cmp(other) <= 0;}
bool Edge::operator> (const Edge* other) const { return this->cmp(other) > 0;}
bool Edge::operator>= (const Edge* other) const { return this->cmp(other) >= 0;}
bool Edge::operator== (const Edge* other) const { return this->cmp(other) == 0; }
bool Edge::operator!= (const Edge* other) const { return this->cmp(other) != 0; }
*/

