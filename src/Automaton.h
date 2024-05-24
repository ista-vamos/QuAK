
#ifndef AUTOMATON_H_
#define AUTOMATON_H_

#include <string>
#include <climits> // INT_MAX
#include <cstring> // memset
#include "Map.h"
#include "Set.h"
#include "State.h"
#include "Symbol.h"


typedef enum {
	Inf,
	Sup,
	LimInf,
	LimSup,
	LimAvg,
} ValueFunction;


struct scc_data_struct;
typedef scc_data_struct scc_data_t;


class Automaton {
private:
	std::string name;
	MapVec<Symbol*>* alphabet;
	MapVec<State*>* states;
	MapVec<scc_data_t*>* SCCs;
	int min_weight;
	int max_weight;
	SetStd<int>* weights;
	State* initial;
	int weight_reachably_recursive (State* state, bool scc_restriction, bool* discovery) const;
	int weight_reachably (State* state, bool scc_restriction) const;
	int weight_safety() const;
	int weight_responce () const;
	int weight_persistance () const;
	unsigned int initialize_SCC_recursive (State* state, int* time, int* discovery, SetList<State*>* SCCs) const;
	void initialize_SCC (void);
	double weight_avg (void);
public:
	Automaton(std::string filename);
	~Automaton();
	bool isDeterministic () const;
	State* getInitial () const;
	std::string toString () const;
	void emptiness ();
};

#endif /* AUTOMATON_H_ */


