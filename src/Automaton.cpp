
#include "Automaton.h"
#include "Parser.h"
#include "Edge.h"
#include "utility.h"




Automaton::~Automaton () {
	delete_verbose("@Memory: Automaton deletion started (automaton %s)\n", this->name.c_str());

	delete_verbose("@Detail: %u Edges will be deleted (automaton %s)\n", this->edges_size, this->name.c_str());
	for (unsigned int state_id = 0; state_id < states->size(); ++state_id) {
		for (auto edge : *(states->at(state_id)->getEdges())) {
			delete edge;
		}
	}

	delete_verbose("@Detail: %u Symbols will be deleted (automaton %s)\n", this->alphabet->size(), this->name.c_str());
	for (unsigned int id = 0; id < this->alphabet->size(); ++id) {
		delete this->alphabet->at(id);
	}
	delete_verbose("@Detail: 1 MapVec (alphabet) will be deleted (automaton %s)\n", this->name.c_str());
	delete alphabet;
	delete_verbose("@Detail: %u States will be deleted (automaton %s)\n", this->states->size(), this->name.c_str());
	for (unsigned int id = 0; id < this->states->size(); ++id) {
		delete this->states->at(id);
	}
	delete_verbose("@Detail: 1 MapVec (states) will be deleted (automaton %s)\n", this->name.c_str());
	delete states;
	delete_verbose("@Detail: %u Weights will be deleted (automaton %s)\n", this->weights->size(), this->name.c_str());
	for (unsigned int id = 0; id < this->weights->size(); ++id) {
		delete this->weights->at(id);
	}
	delete_verbose("@Detail: 1 MapVec (weights) will be deleted (automaton %s)\n", this->name.c_str());
	delete weights;
	delete_verbose("@Detail: 1 SetList (SCCs) will be deleted (automaton %s)\n", this->name.c_str());
	delete this->SCCs;
	delete_verbose("@Memory: Automaton deletion finished (%s)\n", this->name.c_str());
}


Automaton::Automaton (
		std::string name,
		MapVec<Symbol*>* alphabet,
		MapVec<State*>* states,
		MapVec<Weight<weight_t>*>* weights,
		SetList<State*>* SCCs,
		weight_t min_weight,
		weight_t max_weight,
		State* initial,
		unsigned int edges_size
) :
		name(name),
		alphabet(alphabet),
		states(states),
		weights(weights),
		SCCs(SCCs),
		min_weight(min_weight),
		max_weight(max_weight),
		initial(initial),
		edges_size(edges_size)
{}


Automaton::Automaton (std::string filename) :
		name(filename),
		alphabet(NULL),
		states(NULL),
		weights(NULL),
		SCCs(NULL),
		initial(NULL)
{
	Parser parser(filename);
	min_weight = *(parser.weights.begin());
	max_weight = *(parser.weights.begin());
	edges_size = parser.edges.size();

	MapStd<weight_t, Weight<weight_t>*> weight_register;
	MapStd<std::string, Symbol*> symbol_register;
	MapStd<std::string, State*> state_register;

	Weight<weight_t>::RESET();
	this->weights = new MapVec<Weight<weight_t>*>(parser.weights.size());
	Symbol::RESET();
	this->alphabet = new MapVec<Symbol*>(parser.alphabet.size());
	State::RESET();
	this->states = new MapVec<State*>(parser.states.size()+1);// +1 because of INITIAL

	this->initial = new State(parser.initial, alphabet->size());
	this->states->insert(initial->getId(), initial);
	state_register.insert(initial->getName(), initial);
	for (std::string statename : parser.states) {
		State* state = new State(statename, alphabet->size());
		this->states->insert(state->getId(), state);
		state_register.insert(state->getName(), state);
	}

	for (std::string symbolname : parser.alphabet) {
		Symbol* symbol = new Symbol(symbolname);
		this->alphabet->insert(symbol->getId(), symbol);
		symbol_register.insert(symbol->getName(), symbol);
	}

	for (weight_t value : parser.weights) {
		Weight<weight_t>* weight = new Weight<weight_t>(value);
		this->weights->insert(weight->getId(), weight);
		weight_register.insert(weight->getValue(), weight);
		this->min_weight = std::min(this->min_weight, value);
		this->max_weight = std::max(this->max_weight, value);
	}

	for (auto tuple : parser.edges) {
		Symbol* symbol = symbol_register.at(tuple.first.first);
		Weight<weight_t>* weight = weight_register.at(tuple.first.second);
		State* from = state_register.at(tuple.second.first);
		State* to = state_register.at(tuple.second.second);
		Edge *edge = new Edge(symbol, weight, from, to);
		from->addEdge(edge);
		from->addSuccessor(edge);
		to->addPredecessor(edge);
	}

	initialize_SCC();// do not remove without changing ~Automaton
	delete_verbose("@Detail: 3 MapStd will be deleted (automaton constructor registers)\n");
}


State* Automaton::getInitial () const { return initial; }


bool Automaton::isDeterministic () const {
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
			if (1 < this->states->at(state_id)->getSuccessors(symbol_id)->size()) return false;
		}
	}
	return true;
}


void Automaton::initialize_SCC_flood (State* state, weight_t value) const {
	if (state->getTag() > -1) return;
	state->setTag(value);
	for (auto edge : *(state->getEdges())) {
		initialize_SCC_flood(edge->getTo(), value);
	}
}


void Automaton::initialize_SCC_explore (State* state, int* time, int* spot, int* low, SetList<State*>* list) const {
	spot[state->getId()] = *time;
	low[state->getId()] = *time;
	(*time)++;

	for (auto edge : *(state->getEdges())) {
		if (spot[edge->getTo()->getId()] == -1) {
			initialize_SCC_explore(edge->getTo(), time, spot, low, list);
		}
		low[state->getId()] = std::min(low[state->getId()], low[edge->getTo()->getId()]);
	}

	if (low[state->getId()] == spot[state->getId()]) {
		list->push(state);
		initialize_SCC_flood(state, low[state->getId()]);
	}
}

void Automaton::initialize_SCC (void) {
	unsigned int size = this->states->size();
	int spot[size];
	int low[size];
	int time = 0;
	this->SCCs = new SetList<State*>;
	for (unsigned int state_id = 0; state_id < size; ++state_id) {
		spot[state_id] = -1;
	}
	initialize_SCC_explore(initial, &time, spot, low, this->SCCs);
};


weight_t Automaton::weight_reachably_recursive (State* state, bool scc_restriction, bool* discovery) const {
	if (discovery[state->getId()] == true) return min_weight - 1;
	discovery[state->getId()] = true;
	weight_t value = this->min_weight - 1;
	for (auto edge : *(state->getEdges())) {
		if (scc_restriction == false || edge->getTo()->getTag() == state->getTag()) {
			value = std::max(value, edge->getWeight()->getValue());
			value = std::max(value, weight_reachably_recursive(edge->getTo(), scc_restriction, discovery));
		}
	}
	return value;
}

weight_t Automaton::weight_reachably (State* state, bool scc_restriction) const {
	unsigned int size = this->states->size();
	bool discovery[size];
	for (unsigned int state_id = 0; state_id < size; ++state_id) discovery[state_id] = false;
	return weight_reachably_recursive (state, scc_restriction, discovery);
}


weight_t Automaton::weight_responce () const {
	weight_t value = this->min_weight - 1;
	for (auto iter = this->SCCs->cbegin(); iter != this->SCCs->cend(); ++iter) {
		value = std::max(value, weight_reachably(*iter, true));
	}
	return value;
}


void Automaton::toto_handle_edge (Edge* edge, int* values, int** counters) {
	if (values[edge->getFrom()->getId()] <= this->max_weight) {
		printf("\t\tNO HANDLE OF %s\n", edge->toString().c_str());
		return;
	}

	State* state = edge->getFrom();
	counters[state->getId()][edge->getSymbol()->getId()]--;
	printf("\t\tCOUNTER[%u][%u] = %d\n", state->getId(), edge->getSymbol()->getId(), counters[state->getId()][edge->getSymbol()->getId()]);
	if (counters[state->getId()][edge->getSymbol()->getId()] == 0) {
		weight_t max_value = this->min_weight;
		printf("\t\tMAX VALUE: ");
		for (Edge* succ : *(state->getSuccessors(edge->getSymbol()->getId()))) {
			weight_t tmp = std::min(succ->getWeight()->getValue(), values[succ->getTo()->getId()]);
			max_value = std::max(max_value, tmp);
			printf(" %d", max_value);
		}
		printf("\n");
		values[state->getId()] = max_value;
		printf("\t\tVALUE[%u] = %d\n", state->getId(), max_value);

		for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
			for (Edge* pred : *(state->getPredecessors(symbol_id))) {
				toto_handle_edge(pred, values, counters);
			}
		}
	}
};

void Automaton::toto () {
	warning("memory not handled");

	MapVec<SetList<Edge*>*> edges(this->weights->size());
	for (unsigned int weight_id = 0; weight_id < this->weights->size(); ++weight_id) {
		edges.insert(weight_id, new SetList<Edge*>);
	}

	weight_t values[this->states->size()];
	int* counters[this->states->size()];
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		counters[state_id] = new int[this->alphabet->size()];
		values[state_id] = this->max_weight + 1;
		for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
			counters[state_id][symbol_id] = states->at(state_id)->getSuccessors(symbol_id)->size();
			for (Edge* edge : *(this->states->at(state_id)->getSuccessors(symbol_id))) {
				edges.at(edge->getWeight()->getId())->push(edge);
			}
		}
	}

	for (unsigned int weight_id = 0; weight_id < this->weights->size(); ++weight_id) {
		while (edges.at(weight_id)->size() > 0) {
			Edge* edge = edges.at(weight_id)->head();
			edges.at(weight_id)->pop();
			printf("\tTAKE EDGE %s\n", edge->toString().c_str());
			toto_handle_edge(edge, values, counters);
		}
	}

	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		printf("STATE %s TOP VALUE %d\n", this->states->at(state_id)->getName().c_str(), values[state_id]);
	}
}




weight_t Automaton::weight_safety_recursive (State* state, bool scc_restriction, bool* discovery) const {
	if (discovery[state->getId()] == true) return max_weight + 1;
	discovery[state->getId()] = true;
	weight_t min_symbol_value = max_weight + 1;
	for (unsigned int symbol_id = 0; symbol_id < alphabet->size(); ++symbol_id) {
		weight_t max_branching_value = min_weight;
		bool flag = false;
		for (Edge* edge : *(state->getSuccessors(symbol_id))){
			if (scc_restriction == false || edge->getTo()->getTag() == state->getTag()) {
				weight_t tmp = std::min(edge->getWeight()->getValue(), weight_safety_recursive(edge->getTo(), scc_restriction, discovery));
				max_branching_value = std::max (max_branching_value, tmp);
				flag = true;
			}
		}
		if (flag == true) min_symbol_value = std::min(min_symbol_value, max_branching_value);
	}
	return min_symbol_value;
}


weight_t Automaton::weight_safety (State* state, bool scc_restriction) const {
	unsigned int size = this->states->size();
	bool discovery[size];
	for (unsigned int state_id = 0; state_id < size; ++state_id) discovery[state_id] = false;
	return weight_safety_recursive (state, scc_restriction, discovery);
}



weight_t Automaton::weight_persistence () const {
	weight_t value = this->min_weight - 1;
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		weight_t tmp = weight_safety(this->states->at(state_id), true);
		if (tmp > this->max_weight) tmp = this->min_weight - 1;
		value = std::max(value, tmp);
	}
	return value;
}



double Automaton::weight_avg (void) const {
	unsigned int size = this->states->size();
	weight_t distance[size + 1][size];
	weight_t infinity = 1 - (size*(this->min_weight)); //fixme : replacable by a array of initializations

	// O(n)
	for (unsigned int length = 0; length <= size; ++length) {
		for (unsigned int state_id = 0; state_id < size; ++state_id) {
			distance[length][state_id] = infinity;
		}
	}

	// O(n)
	for (auto iter = this->SCCs->cbegin(); iter != this->SCCs->cend(); ++iter) {
		distance[0][(*iter)->getId()] = 0;
	}

	// O(n.m)
	for (unsigned int len = 1; len <= size; ++len) {
		for (unsigned int state_id = 0; state_id < size; ++state_id)	{
			for (auto edge : *(states->at(state_id)->getEdges())) {
				if (edge->getFrom()->getTag() == edge->getTo()->getTag()) {
					if (distance[len-1][edge->getFrom()->getId()] != infinity) {
						weight_t value = distance[len-1][edge->getFrom()->getId()] - edge->getWeight()->getValue();
						if (distance[len][edge->getTo()->getId()] == infinity) {
							distance[len][edge->getTo()->getId()] = value;
						}
						else {
							distance[len][edge->getTo()->getId()] = std::min(value, distance[len][edge->getTo()->getId()]
							);
						}
					}
				}
			}
		}
	}

	//O(n.m)
	double max_state_avg = this->min_weight - 1.0;
	for (unsigned int state_id = 0; state_id < size; ++state_id) {
		double min_lenght_avg = this->max_weight + 1.0;
		bool len_flag = false;
		if (distance[size][state_id] != infinity) { // => id has an ongoing edge (inside its SCC)
			for (unsigned int lenght = 0; lenght < size; ++lenght) { // hence the nested loop is call at most O(m) times
				if (distance[lenght][state_id] != infinity) {
					double avg = (distance[lenght][state_id] - distance[size][state_id] + 0.0) / (size - lenght + 0.0);
					min_lenght_avg = std::min(min_lenght_avg, avg);
					len_flag = true;
				}
			}
		}
		if (len_flag) max_state_avg = std::max(max_state_avg, min_lenght_avg);
	}
	return max_state_avg;
}


double Automaton::computeTop (value_function_t value_function) const {
	switch (value_function) {
		case Inf:
			return weight_safety(this->initial, false);
		case Sup:
			return weight_reachably(this->initial, false);
		case LimInf:
			return weight_persistence();
		case LimSup:
			return weight_responce();
		case LimAvg:
			return weight_avg();
		default:
			fail("automata top");
	}
}



std::string Automaton::top_toString() const {
	weight_t x;
	double y;
	std::string s = "\ttop:";

	x = weight_safety(this->initial, false);
	s.append("\n\t\t   Inf -> ");
	s.append(x>max_weight ? "+infinity" : std::to_string(x));

	x = weight_reachably(this->initial, false);
	s.append("\n\t\t   Sup -> ");
	s.append(x<min_weight ? "-infinity" : std::to_string(x));

	x = weight_persistence();
	s.append("\n\t\tLimInf -> ");
	s.append(x<min_weight ? "+infinity" : std::to_string(x));

	x = weight_responce();
	s.append("\n\t\tLimSup -> ");
	s.append(x<min_weight ? "-infinity" : std::to_string(x));

	y = weight_avg();
	s.append("\n\t\tLimAvg -> ");
	s.append(y<min_weight ? "-infinity" : std::to_string(y));

	s.append("\n");
	return s;
}


std::string Automaton::Automaton::toString (Automaton* A) {
	return A->toString();
}


std::string Automaton::toString () const {
	std::string s = "Automaton \"";
	s.append(this->name);
	s.append("\":\n\t");
	s.append("alphabet:");
	s.append(alphabet->toString(Symbol::toString));
	s.append("\n\t");
	s.append("weights:");
	s.append(weights->toString(Weight<weight_t>::toString));
	s.append("\n\t\tMIN = ");
	s.append(std::to_string(min_weight));
	s.append("\n\t\tMAX = ");
	s.append(std::to_string(max_weight));
	s.append("\n\t");
	s.append("states:");
	s.append(states->toString(State::toString));
	s.append("\n\t\tINITIAL = ");
	s.append(initial->getName());
	s.append("\n\t");
	s.append("edges:");
	for (unsigned int state_id = 0; state_id < states->size(); ++state_id) {
		s.append(states->at(state_id)->getEdges()->toString(Edge::toString));
	}
	s.append("\n");
	s.append(top_toString());
	return s;
}



