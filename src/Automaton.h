
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
protected:
	std::string name;
	MapArray<Symbol*>* alphabet;
	MapArray<State*>* states;
	MapArray<Weight*>* weights;
	weight_t min_domain;
	weight_t max_domain;
	State* initial;
	unsigned int nb_SCCs;
	SCC_Tree* SCCs_tree;

private:
	void build(std::string newname, Parser* parser, MapStd<std::string, Symbol*> sync_register);
	Automaton(std::string newname, Parser* parser, MapStd<std::string, Symbol*> sync_register);
	Automaton(
			std::string name,
			MapArray<Symbol*>* alphabet,
			MapArray<State*>* states,
			MapArray<Weight*>* weights,
			weight_t min_domain,
			weight_t max_domain,
			State* initial
	);
	static Automaton* product(const Automaton* A, aggregator_t aggregator, const Automaton* B);
	static Automaton* constantAutomaton (const Automaton* A, weight_t x);
	static Automaton* booleanize(const Automaton* A, weight_t x);

	bool isDeterministic () const;
	void compute_SCC (void);
	void invert_weights();

	void top_reachably_scc (State* state, bool in_scc, bool* spot, weight_t* values) const;
	void top_reachably_tree (SCC_Tree* tree, bool in_scc, bool* spot, weight_t* values, weight_t* top_values) const;
	weight_t top_Sup (weight_t* top_values) const;
	weight_t top_LimSup (weight_t* top_values) const;
	void top_safety_scc_recursive(Edge* edge, SetStd<Edge*>* done_edge, bool in_scc, int* done_symbol, weight_t* values, weight_t** value_symbol, int** counters) const;
	void top_safety_scc (weight_t* values, bool in_scc) const;
	void top_safety_tree (SCC_Tree* tree, weight_t* values, weight_t* top_values) const;
	weight_t top_Inf (weight_t* top_values) const;
	weight_t top_LimInf (weight_t* top_values) const;
	void top_avg_tree (SCC_Tree* tree, weight_t* top_values) const;
	weight_t top_LimAvg (weight_t* top_values) const;

	bool isIncludedIn_booleanized (const Automaton* B, value_function_t f);
	bool isIncludedIn_antichains (const Automaton* B, value_function_t f);

protected:
	Automaton(const Automaton* A, value_function_t f);
	weight_t compute_Top (value_function_t f, weight_t* top_values) const;
	weight_t compute_Bottom (value_function_t f, weight_t* bot_values);
	void setMaxDomain (weight_t x);
	void setMinDomain (weight_t x);

public:
	~Automaton ();
	Automaton(std::string filename, Automaton* other = nullptr);
	Automaton(std::string filename, value_function_t f, Automaton* other = nullptr);
	static Automaton* determinizeInf (const Automaton* A); // TODO: make private
	static Automaton* copy_trim_complete(const Automaton* A, value_function_t f); // TODO: make private
	static Automaton* from_file_sync_alphabet(std::string filename, Automaton* other = nullptr);
	static Automaton* safetyClosure(const Automaton* A, value_function_t value_function);
	static Automaton* livenessComponent_deterministic (const Automaton* A, value_function_t type);
	static Automaton* livenessComponent_prefixIndependent (const Automaton* A, value_function_t type);
	static Automaton* toLimSup (const Automaton* A, value_function_t f); // TODO: make private
	bool isLimAvgConstant() const; // TODO: make private

	void print () const;
	std::string getName() const;

	bool isEmpty (value_function_t f, weight_t x); 				// checks if A(w) >= v for some w
	bool isUniversal (value_function_t f, weight_t x);			// checks if A(w) >= v for all w
    // checks if A(w) <= B(w) for all w. If `booleanized` is set to true, the inclusion algorithm based
    // on booleanization is used, otherwise the one on anti-chains is used
	bool isIncludedIn (const Automaton* B, value_function_t f, bool booleanized = false);
	bool isSafe (value_function_t f);								// checks if A = SafetyClosure(A)
	bool isConstant (value_function_t f);							// checks if Universal(A, Top_A)
	bool isLive (value_function_t f);								// checks if SafetyClosure(A) = Top_A
	
	weight_t getTopValue (value_function_t f) const;
	weight_t getBottomValue (value_function_t f);
	weight_t getMaxDomain () const;
	weight_t getMinDomain () const;
	State* getInitial () const;
	MapArray<Symbol*>* getAlphabet() const;
	MapArray<State*>* getStates() const;
	MapArray<Weight*>* getWeights() const;
};

#endif /* AUTOMATON_H_ */


