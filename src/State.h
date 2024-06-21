
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
	int my_scc;
	weight_t min_weight;
	weight_t max_weight;
	SetStd<Symbol*>* alphabet;
	SetStd<Edge*>* edges;
	MapArray<SetStd<Edge*>*>* successors;
	MapArray<SetStd<Edge*>*>* predecessors;
public:
	static void RESET();
	~State();
	State (std::string name, unsigned int alphabet_size, weight_t automaton_min_weight, weight_t automaton_max_weight);
	State (std::string name, unsigned int alphabet_size);
	State (State* state);

	std::string getName() const;
	const int getId() const;
	weight_t getMaxWeightValue() const;
	weight_t getMinWeightValue() const;

	const int getTag() const;
	void setTag(int tag);

	SetStd<Symbol*>* getAlphabet () const;

	SetStd<Edge*>* getEdges() const;
	void addEdge (Edge *edge);

	SetStd<Edge*>* getSuccessors(unsigned int symbol_id) const;
	void addSuccessor (Edge* edge);

	SetStd<Edge*>* getPredecessors(unsigned int symbol_id) const;
	void addPredecessor (Edge* edge);

	static std::string toString (State *state);
	std::string toString() const;
	std::string toStringOnlyName() const;
};




#endif /* STATE_H_ */
