
#ifndef AUTOMATON_H_
#define AUTOMATON_H_

#include <string>
#include "Map.h"
#include "Set.h"
#include "Weight.h"
#include "State.h"
#include "Symbol.h"


typedef enum {
	Inf,
	Sup,
	LimInf,
	LimSup,
	LimAvg,
} value_function_t;


struct scc_data_struct;
typedef scc_data_struct scc_data_t;


class Automaton {
private:
	std::string name;
	MapVec<Symbol*>* alphabet;
	MapVec<State*>* states;
	MapVec<Weight<weight_t>*>* weights;
	SetList<State*>* SCCs;
	weight_t min_weight;
	weight_t max_weight;
	State* initial;
	unsigned int edges_size;
private:
	Automaton(
			std::string name,
			MapVec<Symbol*>* alphabet,
			MapVec<State*>* states,
			MapVec<Weight<weight_t>*>* weights,
			SetList<State*>* SCCs,
			weight_t min_weight,
			weight_t max_weight,
			State* initial,
			unsigned int edges_size
	);
	weight_t weight_reachably_recursive (State* state, bool scc_restriction, bool* discovery) const;
	weight_t weight_reachably (State* state, bool scc_restriction) const;
	weight_t weight_safety_recursive (State* state, bool scc_restriction, bool* discovery) const;
	weight_t weight_safety (State* state, bool scc_restriction) const;
	weight_t weight_responce () const;
	weight_t weight_persistence () const;
	void initialize_SCC_flood (State* state, weight_t value) const;
	void initialize_SCC_explore (State* state, int* time, int* discovery, int* low, SetList<State*>* SCCs) const;
	void initialize_SCC (void);
	double weight_avg (void) const;
	std::string top_toString() const;
public:
	void toto ();
	void toto_handle_edge (Edge* edge, weight_t* values, int** counters);
	Automaton (std::string filename);
	~Automaton ();
	bool isDeterministic () const;
	State* getInitial () const;
	double computeTop (value_function_t type) const;
	static std::string toString (Automaton* A);
	std::string toString () const;
};

#endif /* AUTOMATON_H_ */


