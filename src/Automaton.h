
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

typedef enum {
	Max,
	Min,
	Plus,
	Minus,
	Times
} product_weight_t;

class Automaton {
private:
	std::string name;
	MapVec<Symbol*>* alphabet;
	MapVec<State*>* states;
	MapVec<Weight<weight_t>*>* weights;
	SCC_Tree* SCCs_tree;
	//SetList<State*>* SCCs_list; -- NOT NECESSARY
	weight_t min_weight;
	weight_t max_weight;
	State* initial;
	unsigned int nb_reachable_states;
	unsigned int nb_SCCs;
	//int trimmable = 0; -- DO NOT USE THIS
	// -- trimmable cost a loop over all states
	// -- nb_reachable_states cost a single assignment
private:
	Automaton(
			std::string name,
			MapVec<Symbol*>* alphabet,
			MapVec<State*>* states,
			MapVec<Weight<weight_t>*>* weights,
			weight_t min_weight,
			weight_t max_weight,
			State* initial
	);
	void initialize_SCC_flood (State* state, int* tag, int* low, SCC_Tree* ancestor);
	void initialize_SCC_explore (State* state, int *time, int* spot, int* low, SetList<State*>* stack, bool* stackMem);
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
	Automaton (const Automaton& to_copy);
	~Automaton ();
	
	Automaton* safetyClosure(value_function_t value_function) const;
	Automaton* product(value_function_t value_function, const Automaton* B, product_weight_t product_weight) const;
	Automaton* trim() const;
	Automaton* complete(value_function_t value_function) const;
	Automaton* monotonize(value_function_t value_function) const;
	void monotonizeInPlace(value_function_t value_function);
	Automaton* booleanize(Weight<weight_t> v) const;
	Automaton* constantAutomaton (value_function_t type, Weight<weight_t> v) const;
	Automaton* livenessComponent (value_function_t type) const;


	bool isDeterministic () const;
	bool isComplete () const;

	bool isEmpty (value_function_t type, Weight<weight_t> v) const; // checks if A(w) >= v for some w

	bool isUniversal (value_function_t type, Weight<weight_t> v) const; // checks if A(w) >= v for all w
	bool isUniversal_det (value_function_t type, Weight<weight_t> v) const; // checks if A(w) >= v for all w -- assuming deterministic (NEEDS TO BE PROVED)
	
	bool isIncludedIn (value_function_t type, const Automaton* rhs) const; // checks if A(w) <= B(w) for all w
	bool isIncludedIn_det (value_function_t type, const Automaton* rhs) const; // checks if A(w) <= B(w) for all w -- assuming deterministic (this only works for limavg and dsum)
	bool isIncludedIn_bool (value_function_t type, const Automaton* rhs) const;
	
	bool isEquivalent (value_function_t type, const Automaton* rhs) const; // checks if A(w) == B(w) for all w
	
	bool isSafe (value_function_t type) const; // checks if A = SafetyClosure(A)
	
	bool isConstant (value_function_t type) const; // checks if Universal(A, Top_A)
	
	bool isLive (value_function_t type) const; // checks if SafetyClosure(A) = Top_A
	
	State* getInitial () const;
	std::string getName() const;

	static std::string toString (Automaton* A);
	std::string toString () const;
};

#endif /* AUTOMATON_H_ */


