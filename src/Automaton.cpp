
#include "Automaton.h"
#include "Parser.h"
#include "Edge.h"
#include "utility.h"
#include <map>
#include <unordered_map>

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
		for (auto iter = this->nexts->begin(); iter != this->nexts->end(); ++iter) {
			s.append((*iter)->toString(offset));
		}
		return s;
	};
};



Automaton::~Automaton () {
	delete_verbose("@Memory: Automaton deletion started (automaton %s)\n", this->name.c_str());
	// delete_verbose("@Detail: %u Edges will be deleted (automaton %s)\n", this->edges_size, this->name.c_str());
	for (unsigned int state_id = 0; state_id < states->size(); ++state_id) {
		for (Symbol* symbol : *(states->at(state_id)->getAlphabet())) {
			for (Edge* edge : *(states->at(state_id)->getSuccessors(symbol->getId()))) {
				delete edge;
			}
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
	//delete_verbose("@Detail: 1 SetList (SCCs_list) will be deleted (automaton %s)\n", this->name.c_str());
	//delete this->SCCs_list;
	delete_verbose("@Memory: Automaton deletion finished (%s)\n", this->name.c_str());
}








// -------------------------------- Constructors -------------------------------- //


Automaton::Automaton (
		std::string name,
		MapVec<Symbol*>* alphabet,
		MapVec<State*>* states,
		MapVec<Weight<weight_t>*>* weights,
		weight_t min_weight,
		weight_t max_weight,
		State* initial
) :
		name(name),
		alphabet(alphabet),
		states(states),
		weights(weights),
		min_weight(min_weight),
		max_weight(max_weight),
		initial(initial)
{
	initialize_SCC();
}


Automaton::Automaton (std::string filename) :
		name(filename),
		alphabet(NULL),
		states(NULL),
		weights(NULL),
		SCCs_tree(NULL),
		//SCCs_list(NULL),
		initial(NULL)
{
	Parser parser(filename);
	min_weight = *(parser.weights.begin());
	max_weight = *(parser.weights.begin());

	MapStd<weight_t, Weight<weight_t>*> weight_register;
	MapStd<std::string, Symbol*> symbol_register;
	MapStd<std::string, State*> state_register;

	Weight<weight_t>::RESET();
	this->weights = new MapVec<Weight<weight_t>*>(parser.weights.size());
	Symbol::RESET();
	this->alphabet = new MapVec<Symbol*>(parser.alphabet.size());
	State::RESET();
	this->states = new MapVec<State*>(parser.states.size());

	for (std::pair<std::string, unsigned int> pair : parser.states) {
		State* state = new State(pair.first, alphabet->size());
		//unsigned int nb_ougoing_edge = pair.second;
		//MapStd<std::string,unsigned int>* succ = parser.successors.at(pair.first); // symbolname -> nb_succ
		//MapStd<std::string,unsigned int>* pred = parser.predecessors.at(pair.first); // symbolname -> nb_pred
		this->states->insert(state->getId(), state);
		state_register.insert(state->getName(), state);
	}
	this->initial = state_register.at(parser.initial);

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

// TODO: CHECK
// fixme: check if it is necessary, remove otherwise
// so far used in trim and complete
Automaton::Automaton (const Automaton& to_copy) {
	State::RESET();
	Symbol::RESET();
	Weight<weight_t>::RESET();

	this->name = to_copy.name;

	this->alphabet = new MapVec<Symbol*>(to_copy.alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < to_copy.alphabet->size(); ++symbol_id) {
		this->alphabet->insert(symbol_id, new Symbol(to_copy.alphabet->at(symbol_id)));
	}
	this->states = new MapVec<State*>(to_copy.states->size());
	for (unsigned int state_id = 0; state_id < to_copy.states->size(); ++state_id) {
		this->states->insert(state_id, new State(to_copy.states->at(state_id)));
	}
	this->initial = this->states->at(to_copy.initial->getId());
	
	this->weights = new MapVec<Weight<weight_t>*>(to_copy.weights->size());
	this->min_weight = to_copy.min_weight;
	this->max_weight = to_copy.max_weight;
	for (unsigned int weight_id = 0; weight_id < to_copy.weights->size(); ++weight_id) {
		this->weights->insert(weight_id, new Weight<weight_t>(to_copy.weights->at(weight_id)));
	}

	for (unsigned int state_id = 0; state_id < to_copy.states->size(); ++state_id) {
		for (Edge* edge : *(to_copy.states->at(state_id)->getEdges())) {
			Symbol* symbol = this->alphabet->at(edge->getSymbol()->getId());
			State* from = this->states->at(edge->getFrom()->getId());
			State* to = this->states->at(edge->getTo()->getId());
			Weight<weight_t>* weight = to_copy.weights->at(edge->getWeight()->getId());
			Edge* new_edge = new Edge(symbol, weight, from, to);
			from->addEdge(new_edge);
			from->addSuccessor(new_edge);
			to->addPredecessor(new_edge);

		}
	}

	this->initialize_SCC();
}


// TODO: fixme: update to construct only relevant states
Automaton* Automaton::product(value_function_t value_function, const Automaton* B, product_weight_t product_weight) const {
	State::RESET();
	Symbol::RESET();
	Weight<weight_t>::RESET();

	std::string type;
	switch (product_weight) {
		case Max:
			type = "Max";
			break;
		case Min:
			type = "Min";
			break;
		case Plus:
			type = "Plus";
			break;
		case Minus:
			type = "Minus";
			break;
		case Times:
			type = "Times";
			break;
		default:
			fail("automata product weight");
	}
	std::string name = type + "(" + this->getName() + "," + B->getName() + ")";

	MapVec<Symbol*>* alphabet = new MapVec<Symbol*>(this->alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
		alphabet->insert(symbol_id, new Symbol(this->alphabet->at(symbol_id)));
	}

	int n = this->states->size();
	int m = B->states->size();
	// TODO: avoid constructing useless states
	//		n = this->nb_reachable_states
	//		m = B->nb_reachable_states
	MapVec<State*>* states = new MapVec<State*>(n * m);

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			std::string stateName = "(" + this->states->at(i)->getName() + "," + B->states->at(j)->getName() + ")";
			State* pairState = new State(stateName, alphabet->size());
			states->insert(i * m + j, pairState);
		}
	}

	State* initial = states->at(this->initial->getId() * n + B->initial->getId());

	std::map<weight_t,int> counts;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			for (Edge* x : *(this->states->at(i)->getEdges())) {
				for (Edge* y : *(B->states->at(j)->getEdges())) {
					if (x->getSymbol()->getName() == y->getSymbol()->getName()) {
						Weight<weight_t>* pairWeight;
							switch (product_weight) {
								case Max:
									pairWeight = new Weight<weight_t>(std::max(x->getWeight()->getValue(), y->getWeight()->getValue()));
									break;
								case Min:
									pairWeight = new Weight<weight_t>(std::min(x->getWeight()->getValue(), y->getWeight()->getValue()));
									break;
								case Plus:
									pairWeight = new Weight<weight_t>(x->getWeight()->getValue() + y->getWeight()->getValue());
									break;
								case Minus:
									pairWeight = new Weight<weight_t>(x->getWeight()->getValue() - y->getWeight()->getValue());
									break;
								case Times:
									pairWeight = new Weight<weight_t>(x->getWeight()->getValue() * y->getWeight()->getValue());
									break;
								default:
									fail("automata product weight");
							}
							counts[pairWeight->getValue()]++;

						int ii = x->getTo()->getId();
						int jj = y->getTo()->getId();
						Edge* pairEdge = new Edge(x->getSymbol(), pairWeight, states->at(i * m + j), states->at(ii * m + jj));

						states->at(i * m + j)->addEdge(pairEdge);
						states->at(i * m + j)->addSuccessor(pairEdge);
						states->at(ii * m + jj)->addPredecessor(pairEdge);
					}
				}
			}
		}
	}

	MapVec<Weight<weight_t>*>* weights = new MapVec<Weight<weight_t>*>(counts.size());
	weight_t min_weight;
	weight_t max_weight;
	long unsigned int counter = 0;
	for (auto weightCount : counts) {
		Weight<weight_t>* pairWeight = new Weight<weight_t>(weightCount.first);
		weights->insert(counter, pairWeight);

		if (counter == 0) {
			min_weight = weightCount.first;
		}
		if (counter == counts.size() - 1) {
			max_weight = weightCount.first;
		}

		counter++;
	}

	return new Automaton(name, alphabet, states, weights, min_weight, max_weight, initial);
}










// -------------------------------- SCCs -------------------------------- //


void Automaton::initialize_SCC_flood (State* state, int* tag, int* low, SCC_Tree* ancestor) {
	for (Symbol* symbol : *(state->getAlphabet())) {
		for (Edge* edge : *(state->getSuccessors(symbol->getId()))) {
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
}


void Automaton::initialize_SCC_explore (State* state, int* time, int* spot, int* low, SetList<State*>* stack, bool* stackMem) {
	spot[state->getId()] = *time;
	low[state->getId()] = *time;
	(*time)++;
	stack->push(state);
	stackMem[state->getId()] = true;

	for (Symbol* symbol : *(state->getAlphabet())) {
		for (Edge* edge : *(state->getSuccessors(symbol->getId()))) {
			if (spot[edge->getTo()->getId()] == -1) {
				initialize_SCC_explore(edge->getTo(), time, spot, low, stack, stackMem);
				low[state->getId()] = std::min(low[state->getId()], low[edge->getTo()->getId()]);
			}
			else if (stackMem[edge->getTo()->getId()] == true) {
				low[state->getId()] = std::min(low[state->getId()], spot[edge->getTo()-> getId()]);
			}
		}
	}

	if (spot[state->getId()] == low[state->getId()]) {
		//this->SCCs_list->push(state);
		(this->nb_SCCs)++;
		while (stack->head() != state) {
			stackMem[stack->head()->getId()] = false;
			stack->pop();
		}
	}
}


void Automaton::initialize_SCC (void) {
	unsigned int size = this->states->size();
	int* spot = new int[size];
	int* low = new int[size];
	bool* stackMem = new bool[size];
	int time = 0;
	SetList<State*> stack;

	//this->SCCs_list = new SetList<State*>;
	for (unsigned int state_id = 0; state_id < size; ++state_id) {
		spot[state_id] = -1;
		low[state_id] = -1;
		stackMem[state_id] = false;
	}

	initialize_SCC_explore(initial, &time, spot, low, &stack, stackMem);
	this->nb_reachable_states = time;

	int tag = 0;
	this->initial->setTag(0);
	this->SCCs_tree = new SCC_Tree(this->initial);
	this->initialize_SCC_flood(this->initial, &tag, low, this->SCCs_tree);

	delete [] spot;
	delete [] low;
}








// -------------------------------- Getters -------------------------------- //


State* Automaton::getInitial () const { return initial; }


std::string Automaton::getName() const { return this->name; }


// TODO:
// 		check if it is useful
//		if useful then modify the main constructor to maintain Min/Max Weights instead of values
unsigned int Automaton::getMinWeightId() const {
	for (unsigned int i = 0; i < this->weights->size(); i++) {
		if (this->weights->at(i)->getValue() == min_weight) {
			return i;
		}
	}

	return __INT_MAX__;
}


// TODO:
// 		check if it is useful
//		if useful then modify the main constructor to maintain Min/Max Weights instead of values
unsigned int Automaton::getMaxWeightId() const {
	for (unsigned int i = 0; i < this->weights->size(); i++) {
		if (this->weights->at(i)->getValue() == max_weight) {
			return i;
		}
	}

	return __INT_MAX__;
}








// -------------------------------- Tranformations -------------------------------- //


// TODO: Keep until limsup inclusion not implemented, then remove
// --> Inclusion for inf, sup, liminf, limsup done though limsup inclusion
// easy for inf sup liminf limsup. only change the weights to 1 if >= thr, 0 otherwise
// may be nonregular for limavg and dsum
Automaton* Automaton::booleanize(Weight<weight_t> threshold) const {
	State::RESET();
	Symbol::RESET();
	Weight<weight_t>::RESET();

	std::string name = "Bool(" + this->getName() + ", " + threshold.toString() + ")";

	MapVec<Symbol*>* alphabet = new MapVec<Symbol*>(this->alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
		alphabet->insert(symbol_id, new Symbol(this->alphabet->at(symbol_id)));
	}
	MapVec<State*>* states = new MapVec<State*>(this->states->size());
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		states->insert(state_id, new State(this->states->at(state_id)));
	}
	State* initial = states->at(this->initial->getId());

	weight_t min_weight = 1;
	weight_t max_weight = 0;
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		for (Edge* x : *(this->states->at(state_id)->getEdges())) {
			Weight<weight_t>* w;
			if (x->getWeight()->getValue() >= threshold.getValue()) {
				w = new Weight<weight_t>(1);
				max_weight = 1;
			}
			else {
				w = new Weight<weight_t>(0);
				min_weight = 0;
			}
			Edge* e = new Edge(x->getSymbol(), w, states->at(x->getFrom()->getId()), states->at(x->getTo()->getId()));
			states->at(state_id)->addEdge(e);
			states->at(state_id)->addSuccessor(e);
			states->at(x->getTo()->getId())->addPredecessor(e);
		}
	}

	MapVec<Weight<weight_t>*>* weights = new MapVec<Weight<weight_t>*>(max_weight - min_weight + 1);

	for (int i = min_weight; i <= max_weight; i++) {
		Weight<weight_t>* w = new Weight<weight_t>(i);
		weights->insert(i - min_weight, w);
	}

	return new Automaton(name, alphabet, states, weights, min_weight, max_weight, initial);
}


// TODO: remove -- why? we can modify our constructions to not need this, but this is a useful function in general
Automaton* Automaton::trim() const {
	if (this->nb_reachable_states == this->states->size()) {
		return new Automaton(*this);
		// fixme this is terrible, we should really try to keep lost costs
		// i agree but i would do this kind of optimizations at the very end -- we still have other features to add
		// also, making new object could be useful when you want to debug or play around by changing the initial state
	}

	State::RESET();
	Symbol::RESET();
	Weight<weight_t>::RESET();

	std::string name = "Trim(" + this->getName() + ")";

	MapVec<Symbol*>* alphabet = new MapVec<Symbol*>(this->alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
		alphabet->insert(symbol_id, new Symbol(this->alphabet->at(symbol_id)));
	}

	MapVec<State*>* states = new MapVec<State*>(this->nb_reachable_states);
	long unsigned int counter = 0;
	std::unordered_map<unsigned int, unsigned int> stateIdTable;
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		if (this->states->at(state_id)->getTag() > -1) {
			states->insert(counter, new State(this->states->at(state_id)->getName(), this->alphabet->size()));
			stateIdTable[state_id] = counter;
			counter++;
		}
	}
	State* initial = states->at(stateIdTable[this->initial->getId()]);

	std::map<weight_t,int> counts;
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		if (this->states->at(state_id)->getTag() > -1) {
			for (Edge* x : *(this->states->at(state_id)->getEdges())) {
				Weight<weight_t>* w = new Weight<weight_t>(x->getWeight()->getValue());
				Edge* e = new Edge(x->getSymbol(), w, states->at(stateIdTable[x->getFrom()->getId()]), states->at(stateIdTable[x->getTo()->getId()]));
				states->at(stateIdTable[state_id])->addEdge(e);
				states->at(stateIdTable[state_id])->addSuccessor(e);
				states->at(stateIdTable[x->getTo()->getId()])->addPredecessor(e);
				counts[w->getValue()]++;
			}
		}
	}

	MapVec<Weight<weight_t>*>* weights = new MapVec<Weight<weight_t>*>(counts.size());
	weight_t min_weight;
	weight_t max_weight;
	counter = 0;
	for (auto weightCount : counts) {
		Weight<weight_t>* w = new Weight<weight_t>(weightCount.first);
		weights->insert(counter, w);

		if (counter == 0) {
			min_weight = weightCount.first;
		}
		if (counter == counts.size() - 1) {
			max_weight = weightCount.first;
		}

		counter++;
	}

	return new Automaton(name, alphabet, states, weights, min_weight, max_weight, initial);
}


// TODO:
// CHECK if useful
// CHECK if correct
// fixme:
// if useful, change it to change the automaton, not rebuild one
Automaton* Automaton::complete(value_function_t value_function) const {
	if (this->isComplete()) {
		return new Automaton(*this);// fixme: complete should complete the automaton not crating one
		// i agree, but same thing as trim()
	}

	// State::RESET();
	// Symbol::RESET();
	// Weight<weight_t>::RESET();

	unsigned int n = this->states->size();
	unsigned int m = this->alphabet->size();

	bool* outgoing = new bool[n * m];
	for (unsigned int i = 0; i < n * m; i++) {
		outgoing[i] = false;
	}

	std::string name = "Complete(" + this->name + ")";

	MapVec<Symbol*>* alphabet = new MapVec<Symbol*>(this->alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
		alphabet->insert(symbol_id, new Symbol(this->alphabet->at(symbol_id)));
	}
	MapVec<State*>* states = new MapVec<State*>(this->states->size() + 1);
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		states->insert(state_id, new State(this->states->at(state_id)));
	}
	State* initial = states->at(this->initial->getId());
	
	State* sink_state = new State("sink", alphabet->size());
	states->insert(this->states->size(), sink_state);

	MapVec<Weight<weight_t>*>* weights = new MapVec<Weight<weight_t>*>(this->weights->size() + 1);
	weight_t min_weight = this->max_weight;
	weight_t max_weight = this->min_weight;
	for (unsigned int weight_id = 0; weight_id < this->weights->size(); ++weight_id) {
		weights->insert(weight_id, new Weight<weight_t>(this->weights->at(weight_id)));
		min_weight = std::min(min_weight, this->weights->at(weight_id)->getValue());
		max_weight = std::max(max_weight, this->weights->at(weight_id)->getValue());
	}

	// TODO: I assume LimInfAvg, so I guess the undefined values would be +infinity
	weight_t undef;
	if (value_function == Inf || value_function == LimInf || value_function == LimAvg) {
		undef = max_weight + 1;
	}
	else if (value_function == Sup || value_function == LimSup) {
		undef = min_weight - 1;
	}
	else {
		fail("make complete value function");
	}
	Weight<weight_t>* sink_weight = new Weight<weight_t>(undef);
	weights->insert(this->weights->size(), sink_weight);


	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		for (Edge* edge : *(this->states->at(state_id)->getEdges())) {
			outgoing[state_id * m + edge->getSymbol()->getId()] = true;
			Symbol* symbol = alphabet->at(edge->getSymbol()->getId());
			State* from = states->at(edge->getFrom()->getId());
			State* to = states->at(edge->getTo()->getId());
			Weight<weight_t>* weight = weights->at(edge->getWeight()->getId());
			Edge* new_edge = new Edge(symbol, weight, from, to);
			from->addEdge(new_edge);
			from->addSuccessor(new_edge);
			to->addPredecessor(new_edge);

		}
	}

	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
			if (outgoing[state_id * m + symbol_id] == false) {
				Symbol* symbol = alphabet->at(symbol_id);
				State* from = states->at(state_id);
				Edge* sink_edge = new Edge(symbol, sink_weight, from, sink_state);
				from->addEdge(sink_edge);
				from->addSuccessor(sink_edge);
				sink_state->addPredecessor(sink_edge);
				outgoing[state_id * m + symbol_id] = true;
			}
		}
	}

	for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
		Symbol* symbol = alphabet->at(symbol_id);
		Edge* sink_edge = new Edge(symbol, sink_weight, sink_state, sink_state);
		sink_state->addEdge(sink_edge);
		sink_state->addSuccessor(sink_edge);
		sink_state->addPredecessor(sink_edge);
	}

	delete[] outgoing;

	return new Automaton(name, alphabet, states, weights, min_weight, max_weight, initial);
}


Automaton* Automaton::safetyClosure(value_function_t value_function) const {
	State::RESET();
	Symbol::RESET();
	Weight<weight_t>::RESET();

	std::string name = "SafetyClosure(" + this->name + ")";

	MapVec<Symbol*>* alphabet = new MapVec<Symbol*>(this->alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
		alphabet->insert(symbol_id, new Symbol(this->alphabet->at(symbol_id)));
	}
	MapVec<State*>* states = new MapVec<State*>(this->states->size());
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		states->insert(state_id, new State(this->states->at(state_id)));
	}
	State* initial = states->at(this->initial->getId());


	weight_t top_values[this->nb_SCCs];
	computeTop(value_function, top_values);
	MapVec<Weight<weight_t>*>* weights = new MapVec<Weight<weight_t>*>(this->nb_SCCs);
	weight_t min_weight = this->max_weight;
	weight_t max_weight = this->min_weight;
	for (unsigned int weight_id = 0; weight_id < this->nb_SCCs; ++weight_id) {
		weights->insert(weight_id, new Weight<weight_t>(top_values[weight_id]));
		min_weight = std::min(min_weight, top_values[weight_id]);
		max_weight = std::max(max_weight, top_values[weight_id]);
	}

	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		for (Edge* edge : *(this->states->at(state_id)->getEdges())) {
			Symbol* symbol = alphabet->at(edge->getSymbol()->getId());
			State* from = states->at(edge->getFrom()->getId());
			State* to = states->at(edge->getTo()->getId());
			Weight<weight_t>* weight = weights->at(edge->getFrom()->getTag());
			Edge* top_edge = new Edge(symbol, weight, from, to);
			from->addEdge(top_edge);
			from->addSuccessor(top_edge);
			to->addPredecessor(top_edge);
		}
	}

	return new Automaton(name, alphabet, states, weights, min_weight, max_weight, initial);
}


// TODO: CHECK
// Useful for Liveness component of the decomposition
// fixme: make it in place, overall liveness component should build a single automaton
Automaton* Automaton::monotonize (value_function_t type) const {
	if (type != Inf || type != Sup) {
		fail("monotonize only possible for inf or sup automata");
	}

	State::RESET();
	Symbol::RESET();
	Weight<weight_t>::RESET();

	std::string name = "Monotone(" + this->getName() + ")";

	MapVec<Symbol*>* alphabet = new MapVec<Symbol*>(this->alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
		alphabet->insert(symbol_id, new Symbol(this->alphabet->at(symbol_id)));
	}

	unsigned int n = this->states->size();
	unsigned int m = this->weights->size();

	MapVec<State*>* states = new MapVec<State*>(n * m);
	for (unsigned int state_id = 0; state_id < n; ++state_id) {
		for (unsigned int weight_id = 0; weight_id < m; ++weight_id) {
			std::string state_name = "(" + this->states->at(state_id)->toString() + ", " + this->weights->at(weight_id)->toString() + ")";
			State* new_state = new State(state_name, this->alphabet->size());
			states->insert(state_id * n + weight_id, new_state);
		}
	}

	State* initial;
	std::map<weight_t,int> counts;

	if (type == Inf) {
		initial = states->at(this->initial->getId() * m + this->getMaxWeightId());

		for (unsigned int state_id = 0; state_id < n; ++state_id) {
			for (Edge* edge : *(this->states->at(state_id)->getEdges())) {
				Weight<weight_t>* transition_weight = weights->at(edge->getWeight()->getId());
				for (unsigned int weight_id = 0; weight_id < m; ++weight_id) {
					Weight<weight_t>* state_weight = weights->at(weight_id);
					Weight<weight_t>* new_weight = state_weight;
					if (transition_weight->getValue() < state_weight->getValue()) {
						new_weight = transition_weight;
					}

					Symbol* symbol = alphabet->at(edge->getSymbol()->getId());
					State* from = states->at(edge->getFrom()->getId() * m + state_weight->getId());
					State* to = states->at(edge->getTo()->getId() * m + new_weight->getId());
					Edge* new_edge = new Edge(symbol, transition_weight, from, to);
					from->addEdge(new_edge);
					from->addSuccessor(new_edge);
					to->addPredecessor(new_edge);
					counts[transition_weight->getValue()]++;
				}
			}
		}
	}
	else if (type == Sup) {
		initial = states->at(this->initial->getId() * m + this->getMinWeightId());

		for (unsigned int state_id = 0; state_id < n; ++state_id) {
			for (Edge* edge : *(this->states->at(state_id)->getEdges())) {
				Weight<weight_t>* transition_weight = weights->at(edge->getWeight()->getId());
				for (unsigned int weight_id = 0; weight_id < m; ++weight_id) {
					Weight<weight_t>* state_weight = weights->at(weight_id);
					Weight<weight_t>* new_weight = state_weight;
					if (transition_weight->getValue() > state_weight->getValue()) {
						new_weight = transition_weight;
					}

					Symbol* symbol = alphabet->at(edge->getSymbol()->getId());
					State* from = states->at(edge->getFrom()->getId() * m + state_weight->getId());
					State* to = states->at(edge->getTo()->getId() * m + new_weight->getId());
					Edge* new_edge = new Edge(symbol, transition_weight, from, to);
					from->addEdge(new_edge);
					from->addSuccessor(new_edge);
					to->addPredecessor(new_edge);
					counts[transition_weight->getValue()]++;
				}
			}
		}
	}

	MapVec<Weight<weight_t>*>* weights = new MapVec<Weight<weight_t>*>(counts.size());
	weight_t min_weight;
	weight_t max_weight;
	unsigned int counter = 0;
	for (auto weightCount : counts) {
		Weight<weight_t>* w = new Weight<weight_t>(weightCount.first);
		weights->insert(counter, w);

		if (counter == 0) {
			min_weight = weightCount.first;
		}
		if (counter == counts.size() - 1) {
			max_weight = weightCount.first;
		}

		counter++;
	}

	return new Automaton(name, alphabet, states, weights, min_weight, max_weight, initial);
}


// TODO: CHECK
// "this" needs to be monotone and deterministic
// fixme: I don't understand, the liveness component is the copy of the original automaton where
// --> all transitions with the local-top value takes the original min_weight_value -- no they need to take a large value, e.g., the top value
// --> all transitions with NOT the local-top value takes the original max_weight_value -- no they need to take the original weight
Automaton* Automaton::livenessComponent (value_function_t type) const {
	State::RESET();
	Symbol::RESET();
	Weight<weight_t>::RESET();

	std::string name = "LivenessComponent(" + this->getName() + ")";

	MapVec<Symbol*>* alphabet = new MapVec<Symbol*>(this->alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
		alphabet->insert(symbol_id, new Symbol(this->alphabet->at(symbol_id)));
	}
	MapVec<State*>* states = new MapVec<State*>(this->states->size());
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		states->insert(state_id, new State(this->states->at(state_id)));
	}
	State* initial = states->at(this->initial->getId());

	weight_t top_values[this->nb_SCCs];
	computeTop(type, top_values);
	MapVec<Weight<weight_t>*>* weights = new MapVec<Weight<weight_t>*>(this->weights->size());
	weight_t min_weight = this->max_weight;
	weight_t max_weight = this->min_weight;
	for (unsigned int weight_id = 0; weight_id < this->weights->size(); ++weight_id) {
		if (top_values[weight_id] == this->weights->at(weight_id)->getValue()) { // need something like top_values[this->weights->at(weight_id)->getTag()] here
			// if the original transition agrees with the safety closure, liveness component takes a large value (top value of the automaton here)
			weights->insert(weight_id, new Weight<weight_t>(top_values[initial->getId()]));
		}
		else {
			// otherwise, it takes the original transition's weight
			weights->insert(weight_id, new Weight<weight_t>(this->weights->at(weight_id)->getValue()));
		}
		// fixme: min & max are computed with weights that may never occur
		min_weight = std::min(min_weight, weights->at(weight_id)->getValue());
		max_weight = std::max(max_weight, weights->at(weight_id)->getValue());
	}

	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		for (Edge* edge : *(this->states->at(state_id)->getEdges())) {
			Symbol* symbol = alphabet->at(edge->getSymbol()->getId());
			State* from = states->at(edge->getFrom()->getId());
			State* to = states->at(edge->getTo()->getId());
			Weight<weight_t>* weight = weights->at(edge->getWeight()->getId());
			Edge* new_edge = new Edge(symbol, weight, from, to);
			from->addEdge(new_edge);
			from->addSuccessor(new_edge);
			to->addPredecessor(new_edge);
		}
	}

	return new Automaton(name, alphabet, states, weights, min_weight, max_weight, initial);
}



// TODO: CHECK
// fixme:
//		argument type is useless -- dsum would need a slightly different construction if we want to do it
//		check memory leaks
Automaton* Automaton::constantAutomaton (value_function_t type, Weight<weight_t> v) const {
	State::RESET();
	Symbol::RESET();
	Weight<weight_t>::RESET();

	std::string name = "Constant(" + v.toString() + ")";
	
	MapVec<Symbol*>* alphabet = new MapVec<Symbol*>(this->alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
		alphabet->insert(symbol_id, new Symbol(this->alphabet->at(symbol_id)));
	}

	MapVec<State*>* states = new MapVec<State*>(1);
	states->insert(0, new State("init", alphabet->size()));

	State* initial = states->at(0);

	for (unsigned int symbol_id = 0; symbol_id < alphabet->size(); ++symbol_id) {
		Symbol* symbol = alphabet->at(symbol_id);
		State* state = states->at(0);
		Weight<weight_t>* weight = new Weight<weight_t>(v.getValue());
		Edge* edge = new Edge(symbol, weight, state, state);
		state->addEdge(edge);
		state->addSuccessor(edge);
		state->addPredecessor(edge);
	}

	MapVec<Weight<weight_t>*>* weights = new MapVec<Weight<weight_t>*>(1);
	weights->insert(0, new Weight<weight_t>(v.getValue()));

	return new Automaton(name, alphabet, states, weights, v.getValue(), v.getValue(), initial);
}









// -------------------------------- Decisions -------------------------------- //



bool Automaton::isDeterministic () const {
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
			if (1 < this->states->at(state_id)->getSuccessors(symbol_id)->size()) return false;
		}
	}
	return true;
}


// TODO: remove -- why? we don't have to use it ourselves but it is a useful function in general
bool Automaton::isComplete () const {
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
			if (1 > this->states->at(state_id)->getSuccessors(symbol_id)->size()) return false;
		}
	}
	return true;
}


bool Automaton::isEmpty (value_function_t type, Weight<weight_t> v) const {
	weight_t top_values[this->nb_SCCs];
	if (this->computeTop(type, top_values) >= v.getValue()) {
		return false;
	}
	return true;
}



// can update this for DSum (same idea works but not exactly like this)
// fixme: memory leak
bool Automaton::isUniversal (value_function_t type, Weight<weight_t> v) const {
	Automaton* C = this->constantAutomaton(type, v);

	if (C->isIncludedIn(type, this)) {
		return true;
	}

	return false;
}


// TODO: need to prove Bottom(A) = -Top(-A) for limavg (and dsum -- maybe also others)
// fixme: memory leaks
bool Automaton::isUniversal_det (value_function_t type, Weight<weight_t> v) const {
	Weight<weight_t>* minusOne = new Weight<weight_t>(-1);
	Automaton* C = this->constantAutomaton(type, minusOne);

	Automaton* CC = this->product(type, C, Times);
	// TODO:
	// (1) multiply weights by -1 without a product
	// (2) compute top
	// (3) multiply weights by -1 to leave the automaton unchanged
	
	weight_t top_values[CC->nb_SCCs];
	if((-1) * CC->computeTop(type, top_values) >= v.getValue()) {
		return true;
	}

	return false;
}


// this works only for limavg and dsum
// fixme: update product, do not use trim, inline in isIncludedIn case Avg
// fixme: memory leak
bool Automaton::isIncludedIn_det (value_function_t type, const Automaton* rhs) const {
	Automaton* C = this->product(type, rhs, Minus)->trim();
	std::cout << std::endl << C->toString() << std::endl;

	weight_t top_values[C->nb_SCCs];
	weight_t t = C->computeTop(type, top_values);
	if(t < 0) {
		return false;
	}
	return true;
}


// TODO
//		(1) implement translation from inf, sup, liminf to limsup
//		(2) remove
bool Automaton::isIncludedIn_bool(value_function_t type, const Automaton* rhs) const {
	switch (type) {
		case Inf:
			// TODO: safety automata inclusion check (adapt from some tool?)
		case Sup:
			// TODO: reachability automata inclusion check (adapt from some tool?)
		case LimInf:
			// TODO: cobuchi automata inclusion check (adapt from some tool?)
		case LimSup:
			// TODO: buchi automata inclusion check (adapt from some tool?)
		default:
			fail("automata inclusion bool type");
	}
}


// TODO: once limsup inclusion implemented, update to handle all inclusion decisions
// fixme: memory leaks
bool Automaton::isIncludedIn(value_function_t type, const Automaton* rhs) const {
	
	if (type == LimAvg) { 
		if (rhs->isDeterministic()) {
			return this->isIncludedIn_det(type, rhs); // this also works for dsum
		}
		else {
			fail("automata inclusion undecidable for nondeterministic limavg");
		}
	}
	else if (type == Inf || type == Sup || type == LimInf || type == LimSup) {
		for (unsigned int weight_id = 0; weight_id < this->weights->size(); ++weight_id) {
			Automaton* A_bool = this->booleanize(this->weights->at(weight_id));
			Automaton* B_bool = rhs->booleanize(this->weights->at(weight_id));

			if (!A_bool->isIncludedIn_bool(type, B_bool)) {
				return false;
			}
		}
		return true;
	}

	fail("automata inclusion type");
}



// TODO: remove? we only need inclusion test since equivalence are against safety closures
// why? this is a useful function in general
bool Automaton::isEquivalent (value_function_t type, const Automaton* rhs) const {
	return rhs->isIncludedIn(type, this) && this->isIncludedIn(type, rhs);
}


// fixme: memory leak
bool Automaton::isSafe (value_function_t type) const {
	if (this->isDeterministic() || type != LimAvg) {
		return this->isEquivalent(type, this->safetyClosure(type));
		// the safety closure ensure A <= clo(A)
		// fixme: use only inclusion
	}

	// TODO: call constant check for nondet limavg
	// A = this
	// B = det(safe(A))
	// C = Minus(A,B)
	// check if top(C) == 0 and constant(C)
	fail("safety check for nondeterministic limit average is not implemented");
}


// TODO
bool Automaton::isConstant (value_function_t type) const {
	if (type == LimAvg) { 
		if (this->isDeterministic()) {
			weight_t top_values[this->nb_SCCs];
			return this->isUniversal_det(type, this->computeTop(type, top_values));
		}
		else {
			// TODO: implement distance automata boundedness check for nondet limavg
			// https://www.mimuw.edu.pl/~bojan/20152016-2/jezyki-automaty-i-obliczenia-2/distance-automata
			fail("constant check for nondeterministic limit average is not implemented");
		}
	}
	else {
		weight_t top_values[this->nb_SCCs];
		Automaton* Top = this->constantAutomaton(type, this->computeTop(type, top_values));
		return this->isEquivalent(type, Top);
		// fixme: do only what is necessary i.e. universality A <= Top
		// the construction of a constant automaton is the design of isUniversal not isConstant
	}
}


// fixme: memory leaks
bool Automaton::isLive (value_function_t type) const {
	return this->safetyClosure(type)->isConstant(type);
}


// 







// -------------------------------- Tops -------------------------------- //


void Automaton::top_reachably_scc (State* state, lol_t lol, bool* spot, weight_t* values) const {
	if (spot[state->getId()] == true) return;// min_weight - 1;
	spot[state->getId()] = true;
	values[state->getId()] = this->min_weight - 1;
	for (Symbol* symbol : *(state->getAlphabet())) {
		for (Edge* edge : *(state->getSuccessors(symbol->getId()))) {
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
}


void Automaton::top_reachably_tree (SCC_Tree* tree, lol_t lol, bool* spot, weight_t* values, weight_t* top_values) const {
	top_reachably_scc(tree->origin, lol, spot, values);
	top_values[tree->origin->getTag()] = values[tree->origin->getId()];

	for (auto iter = tree->nexts->begin(); iter != tree->nexts->end(); ++iter) {
		top_reachably_tree(*iter, lol, spot, values, top_values);
		top_values[tree->origin->getTag()] = std::max(top_values[tree->origin->getTag()],
				top_values[(*iter)->origin->getTag()]);
	}
}


weight_t Automaton::top_Sup (weight_t* top_values) const {
	weight_t values[this->states->size()];
	bool spot[this->states->size()];
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) spot[state_id] = false;
	top_reachably_tree(this->SCCs_tree, lol_step, spot, values, top_values);
	return top_values[0];
}


weight_t Automaton::top_LimSup (weight_t* top_values) const {
	weight_t values[this->states->size()];
	bool spot[this->states->size()];
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) spot[state_id] = false;
	top_reachably_tree(this->SCCs_tree, lol_in, spot, values, top_values);
	return top_values[0];
}


void Automaton::top_safety_scc_recursive (Edge* edge, bool in_scc, int* values, int** counters) const {
	if (values[edge->getFrom()->getId()] <= this->max_weight) return;

	State* state = edge->getFrom();
	counters[state->getId()][edge->getSymbol()->getId()]--;
	if (counters[state->getId()][edge->getSymbol()->getId()] == 0) {
		weight_t max_value = this->min_weight;
		for (Edge* succ : *(state->getSuccessors(edge->getSymbol()->getId()))) {
			if (in_scc == false || succ->getFrom()->getTag() == succ->getTo()->getTag()) {
				weight_t tmp = std::min(succ->getWeight()->getValue(), values[succ->getTo()->getId()]);
				max_value = std::max(max_value, tmp);
			}
		}
		values[state->getId()] = max_value;

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

	//O((x+m)) because 'top_safety_scc_recursive' is called 2m times overall
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
	for (auto iter = tree->nexts->begin(); iter != tree->nexts->end(); ++iter) {
		top_safety_tree(*iter, top_values);
		top_values[tree->origin->getTag()] = std::max(top_values[tree->origin->getTag()],
				top_values[(*iter)->origin->getTag()]);
	}
}


weight_t Automaton::top_LimInf (weight_t* top_values) const {
	weight_t values[this->states->size()];
	top_safety_scc(values, true);

	for (unsigned int scc_id = 0; scc_id < this->nb_SCCs; ++scc_id) {
		top_values[scc_id] = this->min_weight - 1;
	}

	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		if (values[state_id] <= this->max_weight) {
			top_values[this->states->at(state_id)->getTag()] =
					std::max(top_values[this->states->at(state_id)->getTag()], values[state_id]);
		}

	}

	top_safety_tree(this->SCCs_tree, top_values);
	return top_values[0];
}


void Automaton::top_avg_tree (SCC_Tree* tree, weight_t* top_values) const {
	for (auto iter = tree->nexts->begin(); iter != tree->nexts->end(); ++iter) {
		top_avg_tree(*iter, top_values);
		top_values[tree->origin->getTag()] = std::max(top_values[tree->origin->getTag()],
				top_values[(*iter)->origin->getTag()]);
	}
}


weight_t Automaton::top_LimAvg (weight_t* top_values) const {
	unsigned int size = this->states->size();
	weight_t distance[size + 1][size];
	weight_t infinity = 1 - (size*(this->min_weight));

	// O(n)
	for (unsigned int length = 0; length <= size; ++length) {
		for (unsigned int state_id = 0; state_id < size; ++state_id) {
			distance[length][state_id] = infinity;
		}
	}


	//O(n)
	auto initialize_distances = [] (SCC_Tree* tree, weight_t* t, auto &rec) -> void {
		t[tree->origin->getId()] = 0;
		for (auto iter = tree->nexts->begin(); iter != tree->nexts->end(); ++iter) {
			rec(*iter, t, rec);
		}
	};
	initialize_distances(this->SCCs_tree, distance[0], initialize_distances);

	// -- OLD INITIALIZATION CODE
	//for (auto iter = this->SCCs_list->cbegin(); iter != this->SCCs_list->cend(); ++iter) {
	//	distance[0][(*iter)->getId()] = 0;
	//}

	// O(n.m)
	for (unsigned int len = 1; len <= size; ++len) {
		for (unsigned int state_id = 0; state_id < size; ++state_id)	{
			for (Symbol* symbol : *(states->at(state_id)->getAlphabet())) {
				for (Edge* edge : *(states->at(state_id)->getSuccessors(symbol->getId()))) {
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
	}

	//O(n.m)
	for (unsigned int scc_id = 0; scc_id < this->nb_SCCs; ++scc_id) {
		top_values[scc_id] = this->min_weight - 1;
	}

	for (unsigned int state_id = 0; state_id < size; ++state_id) {
		weight_t min_lenght_avg = this->max_weight + 1.0;
		bool len_flag = false;
		if (distance[size][state_id] != infinity) { // => id has an ongoing edge (inside its SCC)
			for (unsigned int lenght = 0; lenght < size; ++lenght) { // hence the nested loop is call at most O(m) times
				if (distance[lenght][state_id] != infinity) {
					weight_t avg = (distance[lenght][state_id] - distance[size][state_id] + 0.0) / (size - lenght + 0.0);
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
	return top_values[0];
}


weight_t Automaton::computeTop (value_function_t value_function, weight_t* top_values) const {
	switch (value_function) {
		case Inf:
			return top_Inf();
		case Sup:
			return top_Sup(top_values);
		case LimInf:
			return top_LimInf(top_values);
		case LimSup:
			return top_LimSup(top_values);
		case LimAvg:
			return top_LimAvg(top_values);
		default:
			fail("automata top");
	}
}








// -------------------------------- toStrings -------------------------------- //


std::string Automaton::top_toString() const {
	weight_t top_values[this->nb_SCCs];
	weight_t x;
	double y;
	std::string s = "\ttop:";

	x = top_Inf();
	s.append("\n\t\t   Inf -> ");
	s.append(x>max_weight ? "+infinity" : std::to_string(x));

	x = top_Sup(top_values);
	s.append("\n\t\t   Sup -> ");
	s.append(x<min_weight ? "-infinity" : std::to_string(x));

	x = top_LimInf(top_values);
	s.append("\n\t\tLimInf -> ");
	s.append(x<min_weight ? "+infinity" : std::to_string(x));

	x = top_LimSup(top_values);
	s.append("\n\t\tLimSup -> ");
	s.append(x<min_weight ? "-infinity" : std::to_string(x));

	y = top_LimAvg(top_values);
	s.append("\n\t\tLimAvg -> ");
	s.append(y<min_weight ? "-infinity" : std::to_string(y));

	s.append("\n");
	return s;
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
		for (Symbol* symbol : *(states->at(state_id)->getAlphabet())) {
			s.append(states->at(state_id)->getSuccessors(symbol->getId())->toString(Edge::toString));
		}
	}
	// s.append("\n\t");
	// s.append("successors:");
	// for (unsigned int state_id = 0; state_id < states->size(); ++state_id) {
	// 	for (unsigned int symbol_id = 0; symbol_id < alphabet->size(); ++symbol_id) {
	// 		s.append(states->at(state_id)->getSuccessors(symbol_id)->toString(Edge::toString));
	// 	}
	// }
	// s.append("\n\t");
	// s.append("predecessors:");
	// for (unsigned int state_id = 0; state_id < states->size(); ++state_id) {
	// 	for (unsigned int symbol_id = 0; symbol_id < alphabet->size(); ++symbol_id) {
	// 		s.append(states->at(state_id)->getPredecessors(symbol_id)->toString(Edge::toString));
	// 	}
	// }
	s.append("\n");
	s.append(top_toString());
	return s;
}


std::string Automaton::Automaton::toString (Automaton* A) {
	return A->toString();
}



