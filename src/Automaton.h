
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
	unsigned int edges_size;
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
			State* initial,
			unsigned int edges_size
	);
	void print_tree (SCC_Tree* data, std::string offset) const;
	void initialize_SCC_flood (State* state, int* tag, int* low, SCC_Tree* ancestor) const;
	void initialize_SCC_explore (State* state, int* time, int* spot, int* low, SetList<State*>* stack) const;
	void initialize_SCC (void);

	weight_t weight_reachably_recursive_OLD (State* state, bool scc_restriction, bool* spot) const;
	weight_t weight_reachably_OLD (State* state, bool scc_restriction) const;
	weight_t weight_responce_OLD () const;
	void weight_reachably_scc_TMP (State* state, lol_t lol, bool* spot, weight_t* values) const;
	void weight_reachably_tree_TMP (SCC_Tree* tree, lol_t lol, bool* spot, weight_t* values, weight_t* top_values) const;
	weight_t weight_reachably_Sup_TMP () const;
	weight_t weight_reachably_LimSup_TMP () const;

	weight_t weight_safety_recursive_OLD (State* state, bool scc_restriction, bool* discovery) const;
	weight_t weight_safety_OLD (State* state, bool scc_restriction) const;
	weight_t weight_persistence_OLD () const;
	void weight_safety_recursive_TMP (Edge* edge, weight_t* values, int** counters) const;
	void weight_safety_TMP (weight_t* values) const;
	void weight_safety_tree_TMP (SCC_Tree* tree, weight_t* top_values) const;
	weight_t weight_safety_Inf_TMP () const;
	weight_t weight_safety_LimInf_TMP () const;

	void weight_avg_tree_TMP (SCC_Tree* tree, double* top_values) const;
	double weight_avg (void) const;
	std::string top_toString() const;
public:
	Automaton (std::string filename);
	~Automaton ();

	bool isDeterministic () const;
	State* getInitial () const;
	double computeTop (value_function_t type) const;
	static std::string toString (Automaton* A);
	std::string toString () const;
};

#endif /* AUTOMATON_H_ */


