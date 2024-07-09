
#include "Edge.h"
#include "State.h"
#include "utility.h"


Edge::Edge(Symbol* symbol, Weight* weight, State* from, State* to) :
	symbol(symbol), weight(weight), from(from), to(to) {}


Edge::~Edge () {
	delete_verbose("@Memory: Edge deleted (%s)\n", this->toString().c_str());
}


Symbol* Edge::getSymbol() const {
	return this->symbol;
}


Weight* Edge::getWeight() const {
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


