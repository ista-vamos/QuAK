
#include "Edge.h"
#include "utility.h"


Edge::Edge(Symbol* symbol, int weight, State* from, State* to) :
	symbol(symbol), weight(weight), from(from), to(to) {}


Edge::~Edge () {
	delete_verbose("@Memory: Edge deleted\n");
}


Symbol* Edge::getSymbol() const {
	return this->symbol;
}


int Edge::getWeight() const {
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
	s.append(this->symbol->getName());
	s.append(" : ");
	s.append(std::to_string(this->weight));
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

