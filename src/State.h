
#ifndef STATE_H_
#define STATE_H_

#include <string>
#include "Symbol.h"
#include "Set.h"
#include "Map.h"
#include "Edge.h"


class State{
private:
	const int id;
	std::string name;
	int scc_tag;
	SetStd<Edge*>* edges;
	MapVec<SetStd<Edge*>*>* successors;
private:
	void addSuccessor(Edge* edge);
public:
	State (std::string name, unsigned int alphabet_size);
	~State();
	std::string getName() const;
	const int getId() const;
	const int getTag() const;
	void setTag(int tag);
	SetStd<Edge*>* getEdges() const; // fixme : consider removing edges eventually
	MapVec<SetStd<Edge*>*>* getSuccessors() const;
	void addEdge (Edge *edge);
	static std::string toString (State *state);
	std::string toString() const;

	/*
	bool operator< (const State* other) const;
	bool operator<= (const State* other) const;
	bool operator> (const State* other) const;
	bool operator>= (const State* other) const;
	bool operator== (const State* other) const;
	bool operator!= (const State* other) const;
	int cmp (const State* other) const;*/
};






#endif /* STATE_H_ */
