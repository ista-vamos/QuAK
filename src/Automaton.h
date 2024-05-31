
#ifndef AUTOMATON_H_
#define AUTOMATON_H_

#include <string>
#include "Map.h"
#include "Set.h"
#include "Weight.h"
#include "State.h"
#include "Symbol.h"



typedef enum {
	lol_in,
	lol_out,
	lol_step,
} lol_t;


typedef enum {
	Inf,
	Sup,
	LimInf,
	LimSup,
	LimAvg,
} value_function_t;




class Automaton {
private:
	std::string name;
	MapVec<Symbol*>* alphabet;
	MapVec<State*>* states;
	MapVec<Weight<weight_t>*>* weights;
	SCC_Tree* SCCs_tree;
	SetList<State*>* SCCs_list;
	weight_t min_weight;
	weight_t max_weight;
	State* initial;
private:
	Automaton(
			std::string name,
			MapVec<Symbol*>* alphabet,
			MapVec<State*>* states,
			MapVec<Weight<weight_t>*>* weights,
			SCC_Tree* SCCs_tree,
			SetList<State*>* SCCs_list,
			weight_t min_weight,
			weight_t max_weight,
			State* initial
	);
	void initialize_SCC_flood (State* state, int* tag, int* low, SCC_Tree* ancestor) const;
	void initialize_SCC_explore (State* state, int* time, int* spot, int* low, SetList<State*>* stack) const;
	void initialize_SCC (void);

	void top_reachably_scc (State* state, lol_t lol, bool* spot, weight_t* values) const;
	void top_reachably_tree (SCC_Tree* tree, lol_t lol, bool* spot, weight_t* values, weight_t* top_values) const;
	weight_t top_Sup (weight_t* top_values) const;
	weight_t top_LimSup (weight_t* top_values) const;

	void top_safety_scc_recursive(Edge* edge, bool in_scc, weight_t* values, int** counters) const;
	void top_safety_scc (weight_t* values, bool in_scc) const;
	void top_safety_tree (SCC_Tree* tree, weight_t* top_values) const;
	weight_t top_Inf () const;
	weight_t top_LimInf (weight_t* top_values) const;

	void top_avg_tree (SCC_Tree* tree, weight_t* top_values) const;
	weight_t top_LimAvg (weight_t* top_values) const;

	weight_t computeTop (value_function_t value_function, weight_t* top_values) const;
	std::string top_toString() const;
public:
	Automaton (std::string filename);
	~Automaton ();
	Automaton safetyClosure(value_function_t value_function);

	bool isDeterministic () const;
	State* getInitial () const;

	static std::string toString (Automaton* A);
	std::string toString () const;
};

#endif /* AUTOMATON_H_ */


