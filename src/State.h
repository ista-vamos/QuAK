
#ifndef STATE_H_
#define STATE_H_

#include <string>
#include "Set.h"
#include "Map.h"
#include "Edge.h"

class Automaton;

class State{
private:
	const unsigned int my_id;
	std::string name;
  Automaton *automaton{nullptr};
	int my_scc;
	weight_t min_weight;
	weight_t max_weight;
	MapArray<SetStd<Edge*>*>* successors;
	MapArray<SetStd<Edge*>*>* predecessors;

  friend class Automaton;

public:
	static void RESET();
	static void RESET(unsigned int n);
	~State();
	State (std::string name, unsigned int alphabet_size, weight_t automaton_min_weight, weight_t automaton_max_weight);
	State (State* state);

	std::string getName() const;
	unsigned int getId() const;
	weight_t getMaxWeightValue() const;
	weight_t getMinWeightValue() const;

	int getTag() const;
	void setTag(int tag);

	MapArray<Symbol*> *getAlphabet () const;

	SetStd<Edge*>* getSuccessors(unsigned int symbol_id) const;
	void addSuccessor (Edge* edge);

	SetStd<Edge*>* getPredecessors(unsigned int symbol_id) const;
	void addPredecessor (Edge* edge);

	static std::string toString (State *state);
	std::string toString() const;
	//std::string toStringOnlyName() const; // Why not calling get Name?
};




#endif /* STATE_H_ */
