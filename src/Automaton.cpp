
#include "Automaton.h"
#include "Parser.h"
#include "Edge.h"
#include "utility.h"


class SCC_Tree {
public:
	State* origin;
	SetList<SCC_Tree*>* nexts;
	SCC_Tree(State* origin) : origin(origin), nexts(new SetList<SCC_Tree*>) {};
	void addNext (SCC_Tree* next) { this->nexts->push(next); };
	~SCC_Tree() { for (auto tree : *nexts) delete tree; delete nexts; }
	std::string toString (std::string offset) const {
		std::string s = "\n";
		s.append(offset);
		s.append(this->origin->getName());
		offset.append("\t");
		for (auto iter = this->nexts->cbegin(); iter != this->nexts->cend(); ++iter) {
			s.append((*iter)->toString(offset));
		}
		return s;
	};
};



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
	delete_verbose("@Detail: %u SetList (SCC_tree) will be deleted (automaton %s)\n", this->SCCs_list->size(), this->name.c_str());
	delete this->SCCs_tree;
	delete_verbose("@Detail: 1 SetList (SCCs_list) will be deleted (automaton %s)\n", this->name.c_str());
	delete this->SCCs_list;
	delete_verbose("@Memory: Automaton deletion finished (%s)\n", this->name.c_str());
}


Automaton::Automaton (
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
) :
		name(name),
		alphabet(alphabet),
		states(states),
		weights(weights),
		SCCs_tree(SCCs_tree),
		SCCs_list(SCCs_list),
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
		SCCs_tree(NULL),
		SCCs_list(NULL),
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

	initialize_SCC();
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



void Automaton::initialize_SCC_flood (State* state, int* tag, int* low, SCC_Tree* ancestor) const {
	for (auto edge : *(state->getEdges())) {
		if (edge->getTo()->getTag() == -1){
			if (low[state->getId()] == low[edge->getTo()->getId()]) {
				edge->getTo()->setTag(state->getTag());
				initialize_SCC_flood(edge->getTo(), tag, low, ancestor);
			}
			else {
				(*tag)++;
				SCC_Tree* data = new SCC_Tree(edge->getTo());
				edge->getTo()->setTag(*tag);
				initialize_SCC_flood(edge->getTo(), tag, low, data);
				ancestor->addNext(data);
			}
		}
	}
}

void Automaton::initialize_SCC_explore (State* state, int* time, int* spot, int* low, SetList<State*>* stack) const {
	spot[state->getId()] = *time;
	low[state->getId()] = *time;
	stack->push(state);
	(*time)++;
	for (auto edge : *(state->getEdges())) {
		if (spot[edge->getTo()->getId()] == -1) {
			initialize_SCC_explore(edge->getTo(), time, spot, low, stack);
		}
		low[state->getId()] = std::min(low[state->getId()], low[edge->getTo()->getId()]);
	}

	if (spot[state->getId()] == low[state->getId()]) {
		this->SCCs_list->push(state);
		while (stack->head() != state) {
			low[stack->head()->getId()] = low[state->getId()];
			stack->pop();
		}
		stack->pop();
	}
}


void Automaton::initialize_SCC (void) {
	unsigned int size = this->states->size();
	int spot[size];
	int low[size];
	int time = 0;
	SetList<State*> stack;

	this->SCCs_list = new SetList<State*>;
	for (unsigned int state_id = 0; state_id < size; ++state_id) {
		spot[state_id] = -1;
	}
	initialize_SCC_explore(initial, &time, spot, low, &stack);

	int tag = 0;
	initial->setTag(0);
	this->SCCs_tree = new SCC_Tree(initial);
	initialize_SCC_flood(initial, &tag, low, this->SCCs_tree);
};



/*
weight_t Automaton::weight_reachably_recursive_OLD (State* state, bool scc_restriction, bool* spot) const {
	if (spot[state->getId()] == true) return min_weight - 1;
	spot[state->getId()] = true;
	weight_t value = this->min_weight - 1;
	for (auto edge : *(state->getEdges())) {
		if (scc_restriction == false || edge->getTo()->getTag() == state->getTag()) {
			value = std::max(value, edge->getWeight()->getValue());
			value = std::max(value, weight_reachably_recursive_OLD(edge->getTo(), scc_restriction, spot));
		}
	}
	return value;
}

weight_t Automaton::weight_reachably_OLD (State* state, bool scc_restriction) const {
	unsigned int size = this->states->size();
	bool spot[size];
	for (unsigned int state_id = 0; state_id < size; ++state_id) spot[state_id] = false;
	return weight_reachably_recursive_OLD(state, scc_restriction, spot);
}

weight_t Automaton::weight_responce_OLD () const {
	weight_t value = this->min_weight - 1;
	for (auto iter = this->SCCs_list->cbegin(); iter != this->SCCs_list->cend(); ++iter) {
		value = std::max(value, weight_reachably_OLD(*iter, true));
	}
	return value;
}
*/


void Automaton::top_reachably_scc (State* state, lol_t lol, bool* spot, weight_t* values) const {
	if (spot[state->getId()] == true) return;// min_weight - 1;
	spot[state->getId()] = true;
	values[state->getId()] = this->min_weight - 1;
	for (auto edge : *(state->getEdges())) {
		switch (lol) {
			case lol_in:
				if (edge->getTo()->getTag() == state->getTag()) {
					top_reachably_scc(edge->getTo(), lol, spot, values);
					values[state->getId()] = std::max(values[state->getId()], edge->getWeight()->getValue());
					values[state->getId()] = std::max(values[state->getId()], values[edge->getTo()->getId()]);
				}
				break;
			case lol_out:
				top_reachably_scc(edge->getTo(), lol, spot, values);
				values[state->getId()] = std::max(values[state->getId()], edge->getWeight()->getValue());
				values[state->getId()] = std::max(values[state->getId()], values[edge->getTo()->getId()]);
				break;
			case lol_step:
				if (edge->getTo()->getTag() == state->getTag()) {
					top_reachably_scc(edge->getTo(), lol, spot, values);
					values[state->getId()] = std::max(values[state->getId()], values[edge->getTo()->getId()]);
				}
				values[state->getId()] = std::max(values[state->getId()], edge->getWeight()->getValue());
				break;

			default:
				break;
		}
	}
}


void Automaton::top_reachably_tree (SCC_Tree* tree, lol_t lol, bool* spot, weight_t* values, weight_t* top_values) const {
	top_reachably_scc(tree->origin, lol, spot, values);
	top_values[tree->origin->getTag()] = values[tree->origin->getId()];

	for (auto iter = tree->nexts->cbegin(); iter != tree->nexts->cend(); ++iter) {
		top_reachably_tree(*iter, lol, spot, values, top_values);
		top_values[tree->origin->getTag()] = std::max(top_values[tree->origin->getTag()],
				top_values[(*iter)->origin->getTag()]);
	}
}


weight_t Automaton::top_Sup () const {
	weight_t top_values[this->SCCs_list->size()];
	weight_t values[this->states->size()];
	bool spot[this->states->size()];
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) spot[state_id] = false;

	top_reachably_tree(this->SCCs_tree, lol_step, spot, values, top_values);

	printf("TOP SUP ");
	for (auto iter = this->SCCs_list->cbegin(); iter != this->SCCs_list->cend(); ++iter) {
		printf(" (%s, %d)", (*iter)->getName().c_str(), top_values[(*iter)->getTag()]);
	}
	printf("\n");

	return top_values[0];
}

weight_t Automaton::top_LimSup () const {
	weight_t top_values[this->SCCs_list->size()];
	weight_t values[this->states->size()];
	bool spot[this->states->size()];
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) spot[state_id] = false;
	top_reachably_tree(this->SCCs_tree, lol_in, spot, values, top_values);

	printf("TOP LIMSUP ");
	for (auto iter = this->SCCs_list->cbegin(); iter != this->SCCs_list->cend(); ++iter) {
		printf(" (%s, %d)", (*iter)->getName().c_str(), top_values[(*iter)->getTag()]);
	}
	printf("\n");

	return top_values[0];
}



void Automaton::top_safety_scc_recursive (Edge* edge, bool in_scc, int* values, int** counters) const {
	if (values[edge->getFrom()->getId()] <= this->max_weight) {
		tmp_verbose("\t\tNO HANDLE OF %s\n", edge->toString().c_str());
		return;
	}

	State* state = edge->getFrom();
	counters[state->getId()][edge->getSymbol()->getId()]--;
	tmp_verbose("\t\tCOUNTER[%u][%u] = %d\n", state->getId(), edge->getSymbol()->getId(), counters[state->getId()][edge->getSymbol()->getId()]);
	if (counters[state->getId()][edge->getSymbol()->getId()] == 0) {
		weight_t max_value = this->min_weight;
		tmp_verbose("\t\tMAX VALUE: ");
		for (Edge* succ : *(state->getSuccessors(edge->getSymbol()->getId()))) {
			if (in_scc == false || succ->getFrom()->getTag() == succ->getTo()->getTag()) {
				weight_t tmp = std::min(succ->getWeight()->getValue(), values[succ->getTo()->getId()]);
				max_value = std::max(max_value, tmp);
				tmp_verbose(" %d", max_value);
			}
		}
		tmp_verbose("\n");
		values[state->getId()] = max_value;
		tmp_verbose("\t\tVALUE[%u] = %d\n", state->getId(), max_value);

		for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
			for (Edge* pred : *(state->getPredecessors(symbol_id))) {
				if (in_scc == false || pred->getFrom()->getTag() == pred->getTo()->getTag()) {
					top_safety_scc_recursive(pred, in_scc, values, counters);
				}
			}
		}
	}
};

void Automaton::top_safety_scc (weight_t* values, bool in_scc) const {
	//O(x)
	MapVec<SetList<Edge*>*> edges(this->weights->size());
	for (unsigned int weight_id = 0; weight_id < this->weights->size(); ++weight_id) {
		edges.insert(weight_id, new SetList<Edge*>);
	}

	//O(m+n)
	int* counters[this->states->size()];
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		counters[state_id] = new int[this->alphabet->size()];
		values[state_id] = this->max_weight + 1;
		for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
			counters[state_id][symbol_id] = states->at(state_id)->getSuccessors(symbol_id)->size();
			for (Edge* edge : *(this->states->at(state_id)->getSuccessors(symbol_id))) {
				if (in_scc == false || edge->getFrom()->getTag() == edge->getTo()->getTag()){
					edges.at(edge->getWeight()->getId())->push(edge);
				}
			}
		}
	}

	//O((x+m)) because 'toto_handle_edge' is called O(m)
	for (unsigned int weight_id = 0; weight_id < this->weights->size(); ++weight_id) {
		while (edges.at(weight_id)->size() > 0) {
			Edge* edge = edges.at(weight_id)->head();
			edges.at(weight_id)->pop();
			tmp_verbose("\tTAKE EDGE %s\n", edge->toString().c_str());
			top_safety_scc_recursive(edge, in_scc, values, counters);
		}
	}

	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		delete[] counters[state_id];
	}
}


weight_t Automaton::top_Inf () const {
	weight_t values[this->states->size()];
	top_safety_scc(values, false);
	return values[this->initial->getId()];
}



void Automaton::top_safety_tree (SCC_Tree* tree, weight_t* top_values) const {
	for (auto iter = tree->nexts->cbegin(); iter != tree->nexts->cend(); ++iter) {
		top_safety_tree(*iter, top_values);
		top_values[tree->origin->getTag()] = std::max(top_values[tree->origin->getTag()],
				top_values[(*iter)->origin->getTag()]);
	}
}


weight_t Automaton::top_LimInf () const {
	weight_t top_values[this->SCCs_list->size()];
	weight_t values[this->states->size()];
	top_safety_scc(values, true);

	for (unsigned int scc_id = 0; scc_id < this->SCCs_list->size(); ++scc_id) {
		top_values[scc_id] = this->min_weight - 1;
	}

	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		if (values[state_id] <= this->max_weight) {
			top_values[this->states->at(state_id)->getTag()] =
					std::max(top_values[this->states->at(state_id)->getTag()], values[state_id]);
		}

	}

	top_safety_tree(this->SCCs_tree, top_values);

	printf("TOP LIMINF ");
	for (auto iter = this->SCCs_list->cbegin(); iter != this->SCCs_list->cend(); ++iter) {
		printf(" (%s, %d)", (*iter)->getName().c_str(), top_values[(*iter)->getTag()]);
	}
	printf("\n");

	return top_values[0];
}


/*
weight_t Automaton::weight_safety_recursive_OLD (State* state, bool scc_restriction, bool* discovery) const {
	if (discovery[state->getId()] == true) return max_weight + 1;
	discovery[state->getId()] = true;
	weight_t min_symbol_value = max_weight + 1;
	for (unsigned int symbol_id = 0; symbol_id < alphabet->size(); ++symbol_id) {
		weight_t max_branching_value = min_weight;
		bool flag = false;
		for (Edge* edge : *(state->getSuccessors(symbol_id))){
			if (scc_restriction == false || edge->getTo()->getTag() == state->getTag()) {
				weight_t tmp = std::min(edge->getWeight()->getValue(), weight_safety_recursive_OLD(edge->getTo(), scc_restriction, discovery));
				max_branching_value = std::max (max_branching_value, tmp);
				flag = true;
			}
		}
		if (flag == true) min_symbol_value = std::min(min_symbol_value, max_branching_value);
	}
	return min_symbol_value;
}


weight_t Automaton::weight_safety_OLD (State* state, bool scc_restriction) const {
	unsigned int size = this->states->size();
	bool discovery[size];
	for (unsigned int state_id = 0; state_id < size; ++state_id) discovery[state_id] = false;
	return weight_safety_recursive_OLD (state, scc_restriction, discovery);
}



weight_t Automaton::weight_persistence_OLD () const {
	weight_t value = this->min_weight - 1;
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		weight_t tmp = weight_safety_OLD(this->states->at(state_id), true);
		if (tmp > this->max_weight) tmp = this->min_weight - 1;
		value = std::max(value, tmp);
	}
	return value;
}
*/



void Automaton::top_avg_tree (SCC_Tree* tree, double* top_values) const {
	for (auto iter = tree->nexts->cbegin(); iter != tree->nexts->cend(); ++iter) {
		top_avg_tree(*iter, top_values);
		top_values[tree->origin->getTag()] = std::max(top_values[tree->origin->getTag()],
				top_values[(*iter)->origin->getTag()]);
	}
}


double Automaton::top_LimAvg (void) const {
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
	for (auto iter = this->SCCs_list->cbegin(); iter != this->SCCs_list->cend(); ++iter) {
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
	double top_values[this->SCCs_list->size()];
	for (unsigned int i = 0; i < this->SCCs_list->size(); i++) {
		top_values[i] = this->min_weight - 1.0;
	}

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
		if (len_flag) {
			top_values[this->states->at(state_id)->getTag()]
				  = std::max(top_values[this->states->at(state_id)->getTag()], min_lenght_avg);
		}
	}

	top_avg_tree(this->SCCs_tree, top_values);

	printf("TOP AVG ");
	for (auto iter = this->SCCs_list->cbegin(); iter != this->SCCs_list->cend(); ++iter) {
		printf(" (%s, %lf)", (*iter)->getName().c_str(), top_values[(*iter)->getTag()]);
	}
	printf("\n");

	return top_values[0];
}


double Automaton::computeTop (value_function_t value_function) const {
	switch (value_function) {
		case Inf:
			return top_Inf();
		case Sup:
			return top_Sup();
		case LimInf:
			return top_LimInf();
		case LimSup:
			return top_LimSup();
		case LimAvg:
			return top_LimAvg();
		default:
			fail("automata top");
	}
}



std::string Automaton::top_toString() const {
	weight_t x;
	double y;
	std::string s = "\ttop:";

	x = top_Inf();
	s.append("\n\t\t   Inf -> ");
	s.append(x>max_weight ? "+infinity" : std::to_string(x));

	x = top_Sup();
	s.append("\n\t\t   Sup -> ");
	s.append(x<min_weight ? "-infinity" : std::to_string(x));

	x = top_LimInf();
	s.append("\n\t\tLimInf -> ");
	s.append(x<min_weight ? "+infinity" : std::to_string(x));

	x = top_LimSup();
	s.append("\n\t\tLimSup -> ");
	s.append(x<min_weight ? "-infinity" : std::to_string(x));

	y = top_LimAvg();
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
	s.append("SCCs:");
	s.append(this->SCCs_tree->toString("\t\t"));
	s.append("\n\t");
	s.append("edges:");
	for (unsigned int state_id = 0; state_id < states->size(); ++state_id) {
		s.append(states->at(state_id)->getEdges()->toString(Edge::toString));
	}
	s.append("\n");
	s.append(top_toString());
	return s;
}



