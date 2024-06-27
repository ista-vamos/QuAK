
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
	MapArray<Weight*>* weights;
	SCC_Tree* SCCs_tree;
	weight_t min_weight;
	weight_t max_weight;
	State* initial;
	unsigned int nb_reachable_states;// fixme: obsolete (only used in trim)
	unsigned int nb_SCCs;
private:
	static Automaton* build(std::string newname, Parser* parser, MapStd<std::string, Symbol*> sync_register);
	bool isLimAvgConstant() const;
	Automaton(
			std::string name,
			MapArray<Symbol*>* alphabet,
			MapArray<State*>* states,
			MapArray<Weight*>* weights,
			weight_t min_weight,
			weight_t max_weight,
			State* initial
	);
	void compute_SCC (void);

	void top_reachably_scc (State* state, bool in_scc, bool* spot, weight_t* values) const;
	void top_reachably_tree (SCC_Tree* tree, bool in_scc, bool* spot, weight_t* values, weight_t* top_values) const;
	weight_t top_Sup (weight_t* top_values) const;
	weight_t top_LimSup (weight_t* top_values) const;

	void top_safety_scc_recursive(Edge* edge, bool in_scc, weight_t* values, int** counters) const;
	void top_safety_scc (weight_t* values, bool in_scc) const;
	void top_safety_tree (SCC_Tree* tree, weight_t* top_values) const;
	weight_t top_Inf () const;
	weight_t top_LimInf (weight_t* top_values) const;

	void top_avg_tree (SCC_Tree* tree, weight_t* top_values) const;
	weight_t top_LimAvg (weight_t* top_values) const;

	weight_t compute_Top (value_function_t f, weight_t* top_values) const;
	weight_t compute_Bottom (value_function_t f, weight_t* bot_values) const;
	void invert_weights() const;
public:
	static Automaton* trim_and_complete(const Automaton* A, value_function_t f);
	static Automaton* product(const Automaton* A, aggregator_t aggregator, const Automaton* B);
	static Automaton* constantAutomaton (const Automaton* A, weight_t x);
	static Automaton* from_file(std::string filename);
	static Automaton* from_file_sync_alphabet(std::string filename, Automaton* other);
	static Automaton* booleanize(const Automaton* A, weight_t x);
	static Automaton* toLimSup (const Automaton* A, value_function_t f);
	static Automaton* safetyClosure(const Automaton* A, value_function_t value_function);
	static Automaton* livenessComponent (const Automaton* A, value_function_t type);

	~Automaton ();
	void print () const;
	std::string getName() const;

	bool isDeterministic () const;
	bool isEmpty (value_function_t f, weight_t x) const; 				// checks if A(w) >= v for some w
	bool isUniversal (value_function_t f, weight_t x) const;			// checks if A(w) >= v for all w
	bool isIncludedIn (const Automaton* B, value_function_t f) const;	// checks if A(w) <= B(w) for all w
	bool isSafe (value_function_t f) const;								// checks if A = SafetyClosure(A)
	bool isConstant (value_function_t f) const;							// checks if Universal(A, Top_A)
	bool isLive (value_function_t f) const;								// checks if SafetyClosure(A) = Top_A
	
	weight_t getTopValue (value_function_t f) const;
	weight_t getBottomValue (value_function_t f) const;
	weight_t getMaxWeightValue () const;
	weight_t getMinWeightValue () const;
	State* getInitial () const;
	MapArray<Symbol*>* getAlphabet() const;
	MapArray<State*>* getStates() const;
	MapArray<Weight*>* getWeights() const;

};

#endif /* AUTOMATON_H_ */


