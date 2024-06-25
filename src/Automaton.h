
#ifndef AUTOMATON_H_
#define AUTOMATON_H_

#include <string>
#include "Map.h"
#include "Set.h"
#include "Parser.h"
#include "Weight.h"
#include "State.h"
#include "Symbol.h"
#include "Word.h"
#include "FORKLIFT/TargetOf.h"

class SCC_Tree;

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
	LimAvg
} value_function_t;

typedef enum {
	Max,
	Min,
	Plus,
	Minus,
	Times
} aggregator_t;


class Automaton {
private:
	std::string name;
	MapArray<Symbol*>* alphabet;
	MapArray<State*>* states;
	MapArray<Weight<weight_t>*>* weights;
	SCC_Tree* SCCs_tree;
	//SetList<State*>* SCCs_list; -- NOT NECESSARY
	weight_t min_weight;
	weight_t max_weight;
	State* initial;
	unsigned int nb_reachable_states;// fixme: obsolete (only used in trim)
	unsigned int nb_SCCs;
	//int trimmable = 0; -- DO NOT USE THIS
	// -- trimmable cost a loop over all states
	// -- nb_reachable_states cost a single assignment
private:
	void build(Parser* parser, MapStd<std::string, Symbol*> sync_register);
	Automaton(const Automaton* A, value_function_t f);
	Automaton(const Automaton* A, aggregator_t aggregator, const Automaton* B);
	Automaton(
			std::string name,
			MapArray<Symbol*>* alphabet,
			MapArray<State*>* states,
			MapArray<Weight<weight_t>*>* weights,
			weight_t min_weight,
			weight_t max_weight,
			State* initial
	);
	void compute_SCC_tree (State* state, int* spot, int* low, bool* stackMem, SCC_Tree* ancestor);
	void compute_SCC_tag (State* state, int* tag, int *time, int* spot, int* low, SetList<State*>* stack, bool* stackMem);
	void compute_SCC (void);

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

	weight_t compute_Top (value_function_t value_function, weight_t* top_values) const;
	weight_t compute_Bot (value_function_t value_function, weight_t* bot_values) const;

	Automaton* toLimSup_helperLimInf () const;

public:
	Automaton (std::string filename);
	Automaton (std::string filename, Automaton* other);
	//Automaton (const Automaton& to_copy);// fixme: obsolete
	~Automaton ();
	
	Automaton* safetyClosure(value_function_t value_function) const;
	//Automaton* product(value_function_t value_function, const Automaton* B, aggregator_t product_weight) const;// fixme: obsolete
	Automaton* trim();//fixme: to be removed!
	//Automaton* complete(value_function_t value_function) const; // fixme: obsolete
	Automaton* monotonize(value_function_t value_function) const;
	Automaton* booleanize(Weight<weight_t> v) const;
	Automaton* constantAutomaton (Weight<weight_t> v) const;
	Automaton* livenessComponent (value_function_t type) const;
	Automaton* toLimSup (value_function_t type) const;


	bool isDeterministic () const;
	bool isComplete () const;

	bool isEmpty (value_function_t type, Weight<weight_t> v) const; // checks if A(w) >= v for some w

	bool isUniversal (value_function_t type, Weight<weight_t> v) const; // checks if A(w) >= v for all w
	bool isUniversal_det (value_function_t type, Weight<weight_t> v) const; // checks if A(w) >= v for all w -- assuming deterministic
	
	bool isIncludedIn (value_function_t type, const Automaton* rhs) const; // checks if A(w) <= B(w) for all w
	
	bool isEquivalent (value_function_t type, const Automaton* rhs) const; // checks if A(w) == B(w) for all w
	
	bool isSafe (value_function_t type) const; // checks if A = SafetyClosure(A)
	
	bool isConstant (value_function_t type) const; // checks if Universal(A, Top_A)
	
	bool isLive (value_function_t type) const; // checks if SafetyClosure(A) = Top_A
	
	State* getInitial () const;
	MapArray<Symbol*>* getAlphabet() const;
	MapArray<State*>* getStates() const;
	MapArray<Weight<weight_t>*>* getWeights() const;
	weight_t getMinWeightValue () const;
	weight_t getMaxWeightValue () const;
	unsigned int getNbSCCs () const;


	std::string getName() const;

	void print () const;
};

#endif /* AUTOMATON_H_ */


