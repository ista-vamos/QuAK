
#ifndef EDGE_H_
#define EDGE_H_

#include <string>
#include "State.h"
#include "Symbol.h"

class Edge {
private:
	Symbol *symbol;
	int weight;
	State* from;
	State* to;

public:
	Edge(Symbol* symbol, int weight, State* from, State* to);
	~Edge();
	Symbol* getSymbol() const;
	int getWeight() const;
	State *getFrom() const;
	State* getTo() const;

	static std::string toString(Edge* edge);
	std::string toString() const;
/*
	bool operator< (const Edge* other) const;
	bool operator<= (const Edge* other) const;
	bool operator> (const Edge* other) const;
	bool operator>= (const Edge* other) const;
	bool operator== (const Edge* other) const;
	bool operator!= (const Edge* other) const;
	int cmp (const Edge* other) const;*/
};

#endif /* EDGE_H_ */
