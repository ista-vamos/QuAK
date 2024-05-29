
#ifndef STATE_H_
#define STATE_H_

#include <string>
#include "Set.h"
#include "Map.h"
#include "Edge.h"


class State{
private:
	const int my_id;
	std::string name;
	int scc_tag;
	SetStd<Edge*>* edges; // fixme : consider removing edges eventually
	MapVec<SetStd<Edge*>*>* successors;
	MapVec<SetStd<Edge*>*>* predecessors;
public:
	State (std::string name, unsigned int alphabet_size);
	~State();
	static void RESET();
	std::string getName() const;
	const int getId() const;
	const int getTag() const;
	void setTag(int tag);
	SetStd<Edge*>* getEdges() const;
	void addEdge (Edge *edge); // fixme : how to make it private except for Automaton constructors?
	SetStd<Edge*>* getSuccessors(unsigned int symbol_id) const;
	void addSuccessor (Edge* edge);  // fixme : how to make it private except for Automaton constructors?
	SetStd<Edge*>* getPredecessors(unsigned int symbol_id) const;
	void addPredecessor (Edge* edge); // fixme : how to make it private except for Automaton constructors?
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
