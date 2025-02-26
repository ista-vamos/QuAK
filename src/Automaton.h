
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


class SCC_Dag;


typedef enum {
	Inf,
	Sup,
	LimInf,
	LimSup,
	LimInfAvg,
	LimSupAvg,
    // only for monitoring
    Avg,
} value_function_t;

typedef enum {
	Max,
	Min,
	Plus,
	Minus,
	Times
} aggregator_t;

struct UltimatelyPeriodicWord {
    Word* prefix{nullptr};
    Word* cycle{nullptr};
	
	~UltimatelyPeriodicWord() {
		delete prefix;
		delete cycle;
	}
	
	std::string toString() const {
		return prefix->toString() + "(" + cycle->toString() + ")";
	}
};


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
	SCC_Dag** SCCs;

private:
	void build(std::string newname, Parser* parser, MapStd<std::string, Symbol*> sync_register);
	Parser parse_trim();
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
	void compute_SCC (void);
	void appropriateStates();
	void invert_weights();

	void top_dag (SCC_Dag* dag, bool* done, weight_t* top_values) const;
	void top_reachably_scc (State* state, bool in_scc, bool* spot, weight_t* values) const;
	weight_t top_reachably (bool in_scc, weight_t* values, weight_t* top_values) const;
	weight_t top_Sup (weight_t* top_values) const;
	weight_t top_LimSup (weight_t* top_values) const;
	void top_safety_scc_recursive(Edge* edge, SetStd<Edge*>* done_edge, bool in_scc, int* done_symbol, weight_t* values, weight_t** value_symbol, int** counters) const;
	void top_safety_scc (weight_t* values, bool in_scc) const;
	//weight_t top_safety (bool in_scc, weight_t* values, weight_t* top_values) const;
	weight_t top_Inf (weight_t* top_values) const;
	weight_t top_LimInf (weight_t* top_values) const;
	weight_t top_LimAvg (weight_t* top_values) const;

	void constructWitness(value_function_t f, UltimatelyPeriodicWord** witness, const weight_t* scc_values, const weight_t* top_values, SetList<Edge*>** scc_cycles, SetList<Edge*>* path, SetList<Edge*>* loop) const;
	
	weight_t top_LimAvg_cycles (weight_t* top_values, SetList<Edge*>** scc_cycles, UltimatelyPeriodicWord** witness = nullptr) const;
	// State* top_cycles_explore (State* state, bool* spot, weight_t (*filter)(weight_t,weight_t), weight_t* top_values, SetList<Edge*>** scc_cycles) const;
	bool top_cycles_explore (State* target, State* state, bool* spot, weight_t (*filter)(weight_t,weight_t), weight_t* top_values, SetList<Edge*>** scc_cycles) const;
	void top_cycles (weight_t (*filter)(weight_t,weight_t), weight_t* scc_values, weight_t* top_values, SetList<Edge*>** scc_cycles) const;
	weight_t top_LimInf_cycles (weight_t* top_values, SetList<Edge*>** scc_cycles, UltimatelyPeriodicWord** witness = nullptr) const;
	weight_t top_LimSup_cycles (weight_t* top_values, SetList<Edge*>** scc_cycles, UltimatelyPeriodicWord** witness = nullptr) const;
	
	bool top_Sup_witness_explore (weight_t top, State* state, bool* spot, SetList<Edge*>* path) const;
	void top_Sup_witness (weight_t top, SetList<Edge*>* path) const;
	weight_t top_Sup_path (weight_t* top_values, SetList<Edge*>* path, UltimatelyPeriodicWord** witness) const;

	int top_Inf_witness_explore_post (weight_t top, State* state, bool* spot, bool* spot_back, SetList<Edge*>* witness_path, SetList<Edge*>* witness_loop) const;
	bool top_Inf_witness_post (State* init, weight_t top, SetList<Edge*>* witness_path, SetList<Edge*>* witness_loop) const;
	bool top_Inf_witness_explore_pre (weight_t top, State* state, bool* spot, SetList<Edge*>* witness_path, SetList<Edge*>* witness_loop) const;
	void top_Inf_witness (weight_t top, SetList<Edge*>* witness_path, SetList<Edge*>* witness_loop) const;
	weight_t top_Inf_path (weight_t* top_values, SetList<Edge*>* witness_path, SetList<Edge*>* witness_loop, UltimatelyPeriodicWord** witness) const;

	bool isIncludedIn_booleanized (const Automaton* B, value_function_t f, UltimatelyPeriodicWord** witness = nullptr) const;
	bool isIncludedIn_antichains (const Automaton* B, value_function_t f, UltimatelyPeriodicWord** witness = nullptr) const;
  	bool alphabetsAreCompatible(const Automaton *B) const;

	static Automaton* determinizeInf (const Automaton* A);
	bool isLimAvgConstant(UltimatelyPeriodicWord** witness = nullptr) const;

protected:
	Automaton(const Automaton* A, value_function_t f);
	weight_t compute_Top (value_function_t f, weight_t* top_values, UltimatelyPeriodicWord** witness = nullptr) const;
	weight_t compute_Bottom (value_function_t f, weight_t* bot_values, UltimatelyPeriodicWord** witness = nullptr);
	void setMaxDomain (weight_t x);
	void setMinDomain (weight_t x);

public:
	~Automaton ();
	Automaton(std::string filename, Automaton* other = nullptr);
	Automaton(std::string filename, value_function_t f, Automaton* other = nullptr);
	static Automaton* from_file_sync_alphabet(std::string filename, Automaton* other = nullptr);
	static Automaton* safetyClosure(Automaton* A, value_function_t value_function);
	static Automaton* livenessComponent_deterministic (const Automaton* A, value_function_t type);
	static Automaton* livenessComponent_prefixIndependent (const Automaton* A, value_function_t type);
	static Automaton* livenessComponent(const Automaton* A, value_function_t type);
	static Automaton* toLimSup (const Automaton* A, value_function_t f);
	static Automaton* product(const Automaton* A, aggregator_t aggregator, const Automaton* B);

  // Generate a random automaton
  static Automaton *randomAutomaton(const std::string& name,
                                    unsigned states_num,
                                    MapArray<Symbol*>* alphabet,
                                    weight_t min_weight,
                                    weight_t max_weight,
                                    // number of edges, if set to `0`, the number is
                                    // going to be a random number between `states_num / 2` and `states_num*states_num`
                                    // if the automaton should not be complete, and `states_num*alphabet.size()`
                                    // if complete=true
                                    unsigned edges_num=0,
                                      // generate complete automaton?
                                      bool complete = true,
                                    // should we generate exactly `states_num` states
                                    // or _at most_ `states_num` states?
                                    bool states_num_is_max=false);

	static Automaton* constantAutomaton (const Automaton* A, weight_t x);
	static Automaton* booleanize(const Automaton* A, weight_t x);
	static Automaton* copy_trim_complete(const Automaton* A, value_function_t f);

	bool isDeterministic () const;
  	bool isComplete () const;

    /// Print the automaton to stdout
    ///  - `full` print the weights with full precision
    ///  - `bv_weights` print the weights also as bitvectors (unsigned int) type instead of as a floating-point type.
    ///  - `bv_only` print only weights as bitvectors (otherwise they are printed also as a floating-point number).
    ///     Overrides `full` if given.
	void print(bool full = false, bool bv_weights = false, bool bv_only = false) const;
	void print(std::ostream& out, bool full = false, bool bv_weights = false, bool bv_only = false) const;
	void write(std::ostream& out) const;

	const std::string &getName() const;

	bool isNonEmpty (value_function_t f, weight_t x, UltimatelyPeriodicWord** witness = nullptr);	// checks if A(w) >= v for some w
	bool isUniversal (value_function_t f, weight_t x, UltimatelyPeriodicWord** witness = nullptr);	// checks if A(w) >= v for all w
                                                                                                    //
    // checks if A(w) <= B(w) for all w. If `booleanized` is set to true, the inclusion algorithm based
    // on booleanization is used, otherwise the one on anti-chains is used
	bool isIncludedIn (const Automaton* B, value_function_t f, bool booleanized = false, UltimatelyPeriodicWord** witness = nullptr) const;
	bool isEquivalentTo (const Automaton* B, value_function_t f, bool booleanized = false, UltimatelyPeriodicWord** witness = nullptr) const;

	bool isSafe (value_function_t f, UltimatelyPeriodicWord** witness = nullptr);	// checks if A = SafetyClosure(A)
	bool isConstant (value_function_t f, UltimatelyPeriodicWord** witness = nullptr);	// checks if Universal(A, Top_A)
	bool isLive (value_function_t f, UltimatelyPeriodicWord** witness = nullptr);	// checks if SafetyClosure(A) = Top_A
	weight_t getTopValue (value_function_t f, UltimatelyPeriodicWord** witness = nullptr) const;
	weight_t getBottomValue (value_function_t f, UltimatelyPeriodicWord** witness = nullptr);
	weight_t computeValue(value_function_t f, UltimatelyPeriodicWord* w);

	weight_t getMaxDomain () const;
	weight_t getMinDomain () const;
	State* getInitial () const;
	MapArray<Symbol*>* getAlphabet() const;
	MapArray<State*>* getStates() const;
	MapArray<Weight*>* getWeights() const;
	unsigned int getAlphabetSize() const;
};

#endif /* AUTOMATON_H_ */


