
#ifndef EDGE_H_
#define EDGE_H_

#include <string>
#include "Weight.h"
#include "State.h"
#include "Symbol.h"


class Edge {
private:
	Symbol *symbol;
	Weight<weight_t>* weight;
	State* from;
	State* to;

public:
	Edge(Symbol* symbol, Weight<weight_t>* weight, State* from, State* to);
	~Edge();
	Symbol* getSymbol() const;
	Weight<weight_t>* getWeight() const;
	State* getFrom() const;
	State* getTo() const;

	static std::string toString(Edge* edge);
	std::string toString() const;
};

#endif /* EDGE_H_ */
