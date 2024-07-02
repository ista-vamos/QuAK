
#include "Automaton.h"
#include "Parser.h"
#include "Edge.h"
#include "utility.h"
#include "FORKLIFT/inclusion.h"


class SCC_Tree {
public:
	State* origin;
	SetList<SCC_Tree*>* nexts;
	SCC_Tree(State* origin) : origin(origin), nexts(new SetList<SCC_Tree*>) {};
	SCC_Tree(State* origin, SetList<SCC_Tree*>* nexts) : origin(origin), nexts(nexts) {};
	void addNext (SCC_Tree* next) { this->nexts->push(next); };
	~SCC_Tree() { for (auto tree : *nexts) delete tree; delete nexts; }
	std::string toString (std::string offset) const {
		std::string s = "\n";
		s.append(offset);
		s.append(this->origin->getName());
		offset.append("\t");
		for (auto tree : *nexts) s.append(tree->toString(offset));
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
	for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
		delete this->alphabet->at(symbol_id);
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
	// delete_verbose("@Detail: %u SetList (SCC_tree) will be deleted (automaton %s)\n", this->SCCs_list->size(), this->name.c_str());
	delete this->SCCs_tree;
	delete_verbose("@Memory: Automaton deletion finished (%s)\n", this->name.c_str());
}








// -------------------------------- Constructors -------------------------------- //


Automaton::Automaton (
		std::string name,
		MapArray<Symbol*>* alphabet,
		MapArray<State*>* states,
		MapArray<Weight*>* weights,
		weight_t min_domain,
		weight_t max_domain,
		State* initial
) :
		name(name),
		alphabet(alphabet),
		states(states),
		weights(weights),
		min_domain(min_domain),
		max_domain(max_domain),
		initial(initial)
{
	compute_SCC();
}


void Automaton::build(std::string newname, Parser* parser, MapStd<std::string, Symbol*> sync_register){
	this->name = newname;

	Symbol::RESET(sync_register.size());
	State::RESET();
	Weight::RESET();

	MapStd<weight_t, Weight*> weight_register;
	MapStd<std::string, State*> state_register;
	MapStd<std::string, Symbol*> symbol_register;

	this->min_domain = parser->min_domain;
	this->max_domain = parser->max_domain;

	this->weights = new MapArray<Weight*>(parser->weights.size());
	this->alphabet = new MapArray<Symbol*>(parser->alphabet.size());
	this->states = new MapArray<State*>(parser->states.size());

	for (weight_t value : parser->weights) {
		Weight* weight = new Weight(value);
		this->weights->insert(weight->getId(), weight);
		weight_register.insert(weight->getValue(), weight);
	}


	for (std::string statename : parser->states) {
		State* state = new State(statename, this->alphabet->size(), this->min_domain, this->max_domain);
		this->states->insert(state->getId(), state);
		state_register.insert(state->getName(), state);
	}
	this->initial = state_register.at(parser->initial);

	for (std::string symbolname : parser->alphabet) {
		Symbol * symbol;
		if (sync_register.contains(symbolname))
			symbol = new Symbol(sync_register.at(symbolname));
		else
			symbol = new Symbol(symbolname);
		this->alphabet->insert(symbol->getId(), symbol);
		symbol_register.insert(symbol->getName(), symbol);
	}

	for (auto tuple : parser->edges) {
		Symbol* symbol = symbol_register.at(tuple.first.first);
		Weight* weight = weight_register.at(tuple.first.second);
		State* from = state_register.at(tuple.second.first);
		State* to = state_register.at(tuple.second.second);
		Edge *edge = new Edge(symbol, weight, from, to);
		from->addSuccessor(edge);
		to->addPredecessor(edge);
	}

	compute_SCC();
}



Automaton::Automaton(std::string newname, Parser* parser, MapStd<std::string, Symbol*> sync_register) {
	build(newname, parser, sync_register);
}

Automaton::Automaton(std::string filename, Automaton* other) {
	MapStd<std::string, Symbol*> sync_register;
	if (other != nullptr) {
		for (unsigned int symbol_id = 0; symbol_id < other->alphabet->size(); ++symbol_id) {
			other->alphabet->at(symbol_id);
			sync_register.insert(other->alphabet->at(symbol_id)->getName(), other->alphabet->at(symbol_id));
		}
	}
	Parser parser(filename, &sync_register);
	build(filename, &parser, sync_register);
}


Automaton* Automaton::from_file_sync_alphabet (std::string filename, Automaton* other) {
	MapStd<std::string, Symbol*> sync_register;
	if (other != nullptr) {
		for (unsigned int symbol_id = 0; symbol_id < other->alphabet->size(); ++symbol_id) {
			other->alphabet->at(symbol_id);
			sync_register.insert(other->alphabet->at(symbol_id)->getName(), other->alphabet->at(symbol_id));
		}
	}
	Parser parser(filename, &sync_register);
	return (new Automaton(filename, &parser, sync_register));
}



std::string aggregator_name (aggregator_t aggregator) {
	switch (aggregator) {
		case Max: return "Max";
		case Min: return "Min";
		case Plus: return "Plus";
		case Minus: return "Minus";
		case Times: return "Times";
		default: fail("case aggregator_t");
	}
}

weight_t aggregator_apply (aggregator_t aggregator, Weight* x, Weight* y) {
	switch (aggregator) {
		case Max: return std::max(x->getValue(), y->getValue());
		case Min: return std::min(x->getValue(), y->getValue());
		case Plus: return x->getValue() + y->getValue();
		case Minus: return x->getValue() - y->getValue();
		case Times: return x->getValue() * y->getValue();
		default: fail("case aggregator_t");
	}
}


Automaton* Automaton::product(const Automaton* A, aggregator_t f, const Automaton* B) {
	MapStd<std::string, Symbol*> sync_register;
	Parser parser(std::min(A->min_domain, B->min_domain), std::max(A->max_domain, B->max_domain));

	for (unsigned int stateA_id = 0; stateA_id < A->states->size(); ++stateA_id) {
		if (A->states->at(stateA_id)->getTag() == -1) continue;
		for (unsigned int stateB_id = 0; stateB_id < B->states->size(); ++stateB_id) {
			if (B->states->at(stateB_id)->getTag() == -1) continue;
			for (Symbol* symbol : *(A->states->at(stateA_id)->getAlphabet())) {
				if (B->alphabet->size() <= symbol->getId()) continue;
				if (B->alphabet->at(symbol->getId())->getName() != symbol->getName()) fail("product with unsynchronized alphabet");

				for (Edge* edgeA : *(A->states->at(stateA_id)->getSuccessors(symbol->getId()))) {
					for (Edge* edgeB : *(B->states->at(stateB_id)->getSuccessors(symbol->getId()))) {
						std::string symbolname = symbol->getName();
						weight_t weightvalue = aggregator_apply(f, edgeA->getWeight(), edgeB->getWeight());
						std::string fromname =  "(" + edgeA->getFrom()->getName() + "," + edgeB->getFrom()->getName() + ")";
						std::string toname =  "(" + edgeA->getTo()->getName() + "," + edgeB->getTo()->getName() + ")";

						parser.alphabet.insert(symbolname);
						parser.weights.insert(weightvalue);
						parser.states.insert(fromname);
						parser.states.insert(toname);

						std::pair<std::pair<std::string, weight_t>,std::pair<std::string, std::string>> edge;
						edge.first.first = symbolname;
						edge.first.second = weightvalue;
						edge.second.first = fromname;
						edge.second.second = toname;
						parser.edges.insert(edge);
					}
				}
			}
		}
	}

	std::string newname =  aggregator_name(f) + "(" + A->getName() + "," + B->getName() + ")";
	return (new Automaton(newname, &parser, sync_register));
}



Parser* parse_trim_complete(const Automaton* A, value_function_t f) {
	Parser* parser = new Parser(A->getMinDomain(), A->getMaxDomain());

	for (unsigned int stateA_id = 0; stateA_id < A->getStates()->size(); ++stateA_id) {
		if (A->getStates()->at(stateA_id)->getTag() == -1) continue;
		parser->states.insert(A->getStates()->at(stateA_id)->getName());
		for (Symbol* symbol : *(A->getStates()->at(stateA_id)->getAlphabet())) {
			parser->alphabet.insert(symbol->getName());
			for (Edge* edgeA : *(A->getStates()->at(stateA_id)->getSuccessors(symbol->getId()))) {
				parser->weights.insert(edgeA->getWeight()->getValue());
				std::pair<std::pair<std::string, weight_t>,std::pair<std::string, std::string>> edge;
				edge.first.first = edgeA->getSymbol()->getName();
				edge.first.second = edgeA->getWeight()->getValue();
				edge.second.first = edgeA->getFrom()->getName();
				edge.second.second = edgeA->getTo()->getName();
				parser->edges.insert(edge);
			}
		}
	}

	weight_t sinkvalue;
	switch(f) {
		case Inf: case LimInf : case LimAvg:
			sinkvalue = A->getMaxDomain();
			break;
		case Sup: case LimSup:
			sinkvalue = A->getMinDomain();
			break;
		default: fail("case value function");
	}

	for (unsigned int stateA_id = 0; stateA_id < A->getStates()->size(); ++stateA_id) {
		if (A->getStates()->at(stateA_id)->getTag() == -1) continue;
		for (unsigned int symbol_id = 0; symbol_id < A->getStates()->size(); ++symbol_id) {
			if (parser->alphabet.contains(A->getAlphabet()->at(symbol_id)->getName()) == false) continue;
			if (A->getStates()->at(stateA_id)->getAlphabet()->contains(A->getAlphabet()->at(symbol_id)) == true) continue;
			parser->states.insert("#sink#");
			parser->weights.insert(sinkvalue);
			std::pair<std::pair<std::string, weight_t>,std::pair<std::string, std::string>> edge;
			edge.first.first = A->getAlphabet()->at(symbol_id)->getName();
			edge.first.second = sinkvalue;
			edge.second.first = A->getStates()->at(stateA_id)->getName();
			edge.second.second = "#sink#";
			parser->edges.insert(edge);
		}
	}

	for (std::string symbolname: parser->alphabet) {
		std::pair<std::pair<std::string, weight_t>,std::pair<std::string, std::string>> edge;
		edge.first.first = symbolname;
		edge.first.second = sinkvalue;
		edge.second.first = "#sink#";
		edge.second.second = "#sink#";
		parser->edges.insert(edge);
	}

	return parser;
}


Automaton::Automaton(const Automaton* A, value_function_t f) {
	MapStd<std::string, Symbol*> sync_register;
	Parser* parser = parse_trim_complete(A, f);
	build(A->name, parser, sync_register);
	delete parser;
}


Automaton* Automaton::copy_trim_complete(const Automaton* A, value_function_t f) {
	MapStd<std::string, Symbol*> sync_register;
	Parser* parser = parse_trim_complete(A, f);
	Automaton* that = new Automaton(A->name, parser, sync_register);
	delete parser;
	return that;
}



// -------------------------------- SCCs -------------------------------- //


void compute_SCC_tree (State* state, int* spot, int* low, bool* stackMem, SCC_Tree* ancestor) {
	stackMem[state->getId()] = true;

	for (Symbol* symbol : *(state->getAlphabet())) {
		for (Edge* edge : *(state->getSuccessors(symbol->getId()))) {
			if (stackMem[edge->getTo()->getId()] == false) {
				if (spot[edge->getTo()->getId()] == low[edge->getTo()->getId()]) {
					SCC_Tree* data = new SCC_Tree(edge->getTo());
					ancestor->addNext(data);
					compute_SCC_tree(edge->getTo(), spot, low, stackMem, data);
				}
				else {
					compute_SCC_tree(edge->getTo(), spot, low, stackMem, ancestor);
				}
			}
		}
	}
}


void compute_SCC_tag (State* state, int* tag, int* time, int* spot, int* low, SetList<State*>* stack, bool* stackMem) {
	spot[state->getId()] = *time;
	low[state->getId()] = *time;
	(*time)++;
	stack->push(state);
	stackMem[state->getId()] = true;

	for (Symbol* symbol : *(state->getAlphabet())) {
		for (Edge* edge : *(state->getSuccessors(symbol->getId()))) {
			if (spot[edge->getTo()->getId()] == -1) {
				compute_SCC_tag(edge->getTo(), tag, time, spot, low, stack, stackMem);
				low[state->getId()] = std::min(low[state->getId()], low[edge->getTo()->getId()]);
			}
			else if (stackMem[edge->getTo()->getId()] == true) {
				low[state->getId()] = std::min(low[state->getId()], spot[edge->getTo()-> getId()]);
			}
		}
	}

	if (spot[state->getId()] == low[state->getId()]) {
		while (stack->head() != state) {
			stack->head()->setTag(*tag);
			stackMem[stack->head()->getId()] = false;
			stack->pop();
		}
		state->setTag(*tag);
		(*tag)++;
		stackMem[state->getId()] = false;
		stack->pop();
	}
}


void Automaton::compute_SCC (void) {
	unsigned int size = this->states->size();
	int* spot = new int[size];
	int* low = new int[size];
	bool* stackMem = new bool[size];
	int time = 0;
	int tag = 0;
	SetList<State*> stack;

	for (unsigned int state_id = 0; state_id < size; ++state_id) {
		spot[state_id] = -1;
		stackMem[state_id] = false;
	}

	compute_SCC_tag(initial, &tag, &time, spot, low, &stack, stackMem);
	this->nb_SCCs = tag;

	this->SCCs_tree = new SCC_Tree(this->initial);
	compute_SCC_tree(this->initial, spot, low, stackMem, this->SCCs_tree);

	delete [] spot;
	delete [] low;
	delete [] stackMem;
}








// -------------------------------- Getters -------------------------------- //

weight_t Automaton::getTopValue (value_function_t f) const {
	weight_t top_values[this->nb_SCCs];
	return compute_Top(f, top_values);

}

weight_t Automaton::getBottomValue (value_function_t f) const {
	weight_t bot_values[this->nb_SCCs];
	return compute_Bottom(f, bot_values);
}


MapArray<Symbol*>* Automaton::getAlphabet() const { return this->alphabet; }
MapArray<State*>* Automaton::getStates() const { return this->states; }
MapArray<Weight*>* Automaton::getWeights() const { return this->weights; }
weight_t Automaton::getMinDomain () const { return this->min_domain; }
weight_t Automaton::getMaxDomain () const { return this->max_domain; }
State* Automaton::getInitial () const { return initial; }
std::string Automaton::getName() const { return this->name; }



void Automaton::invert_weights() const {
	for (unsigned int weight_id = 0; weight_id < this->weights->size(); ++weight_id) {
		weight_t value = this->weights->at(weight_id)->getValue();
		this->weights->at(weight_id)->setValue(-value);
	}

	unsigned int i, j;
	for (i = 0, j = this->weights->size()-1; i < j; ++i, --j) {
		Weight* weight_at_i = this->weights->at(i);
		Weight* weight_at_j = this->weights->at(j);
		this->weights->insert(i, weight_at_j);
		this->weights->insert(j, weight_at_i);
	}
}







// -------------------------------- Tranformations -------------------------------- //



Automaton* Automaton::constantAutomaton (const Automaton* A, weight_t x) {
	State::RESET();
	Symbol::RESET();
	Weight::RESET();

	std::string newname = "Constant(" + std::to_string(x) + ")";

	MapArray<Symbol*>* newalphabet = new MapArray<Symbol*>(A->alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < A->alphabet->size(); ++symbol_id) {
		newalphabet->insert(symbol_id, new Symbol(A->alphabet->at(symbol_id)));
	}

	MapArray<State*>* newstates = new MapArray<State*>(1);
	newstates->insert(0, new State("unique", newalphabet->size(), x, x));
	State* newinitial = newstates->at(0);

	Weight* weight = new Weight(x);
	MapArray<Weight*>* newweights = new MapArray<Weight*>(1);
	newweights->insert(0, weight);

	for (unsigned int symbol_id = 0; symbol_id < newalphabet->size(); ++symbol_id) {
		State* state = newstates->at(0);
		Edge* edge = new Edge(newalphabet->at(symbol_id), weight, state, state);
		state->addSuccessor(edge);
		state->addPredecessor(edge);
	}

	return new Automaton(newname, newalphabet, newstates, newweights, x, x, newinitial);
}



Automaton* Automaton::booleanize(const Automaton* A, weight_t x) {
	State::RESET();
	Symbol::RESET();
	Weight::RESET();

	std::string newname = "Bool(" + A->getName() + ", " + std::to_string(x) + ")";

	MapArray<Symbol*>* newalphabet = new MapArray<Symbol*>(A->alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < A->alphabet->size(); ++symbol_id) {
		newalphabet->insert(symbol_id, new Symbol(A->alphabet->at(symbol_id)));
	}

	MapArray<State*>* newstates = new MapArray<State*>(A->states->size());
	for (unsigned int state_id = 0; state_id < A->states->size(); ++state_id) {
		newstates->insert(state_id, new State(A->states->at(state_id)));
	}
	State* newinitial = newstates->at(A->initial->getId());

	MapArray<Weight*>* newweights = new MapArray<Weight*>(2);
	for (unsigned int weight_id = 0; weight_id < 2; ++weight_id) {
		newweights->insert(weight_id, new Weight(weight_id));
	}

	for (unsigned int state_id = 0; state_id < A->states->size(); ++state_id) {
		for (Symbol* symbol : *(A->states->at(state_id)->getAlphabet())) {
			for (Edge* edge : *(A->states->at(state_id)->getSuccessors(symbol->getId()))) {
				weight_t value = ((edge->getWeight()->getValue() >= x) ? 1 : 0);
				Weight* weight = newweights->at(value);
				State* from = newstates->at(edge->getFrom()->getId());
				State* to = newstates->at(edge->getTo()->getId());
				Edge* newedge = new Edge(newalphabet->at(symbol->getId()), weight, from, to);
				newstates->at(state_id)->addSuccessor(newedge);
				newstates->at(edge->getTo()->getId())->addPredecessor(newedge);
			}
		}
	}

	return new Automaton(newname, newalphabet, newstates, newweights, 0, 1, newinitial);
}



Automaton* Automaton::safetyClosure(const Automaton* A, value_function_t f) {
	State::RESET();
	Symbol::RESET();
	Weight::RESET();

	std::string newname = "SafeOf(" + A->name + ")";

	MapArray<Symbol*>* newalphabet = new MapArray<Symbol*>(A->alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < A->alphabet->size(); ++symbol_id) {
		newalphabet->insert(symbol_id, new Symbol(A->alphabet->at(symbol_id)));
	}
	MapArray<State*>* newstates = new MapArray<State*>(A->states->size());
	for (unsigned int state_id = 0; state_id < A->states->size(); ++state_id) {
		newstates->insert(state_id, new State(A->states->at(state_id)));
	}
	State* newinitial = newstates->at(A->initial->getId());

	weight_t top_values[A->nb_SCCs];
	A->compute_Top(f, top_values);

	SetSorted<weight_t> weight_values;
	MapStd<weight_t, Weight*> weight_register;
	for (unsigned int scc_id = 0; scc_id < A->nb_SCCs; ++scc_id) {
		weight_values.insert(top_values[scc_id]);
	}
	weight_t newmin_domain = A->min_domain;//domain does not change
	weight_t newmax_domain = A->max_domain;//domain does not depend on weights

	MapArray<Weight*>* newweights = new MapArray<Weight*>(weight_values.size());
	for (weight_t value : weight_values) {
		Weight* weight = new Weight(value);
		newweights->insert(weight->getId(), weight);
		weight_register.insert(weight->getValue(), weight);
	}

	for (unsigned int state_id = 0; state_id < A->states->size(); ++state_id) {
		for (unsigned int symbol_id = 0; symbol_id < A->alphabet->size(); ++symbol_id) {
			for (Edge* edge : *(A->states->at(state_id)->getSuccessors(symbol_id))) {
				Symbol* symbol = newalphabet->at(edge->getSymbol()->getId());
				State* from = newstates->at(edge->getFrom()->getId());
				State* to = newstates->at(edge->getTo()->getId());
				Weight* weight = weight_register.at(top_values[edge->getFrom()->getTag()]);
				Edge* newedge = new Edge(symbol, weight, from, to);
				from->addSuccessor(newedge);
				to->addPredecessor(newedge);
			}
		}
	}

	return new Automaton(newname, newalphabet, newstates, newweights, newmin_domain, newmax_domain, newinitial);
}



Automaton* Automaton::livenessComponent (const Automaton* A, value_function_t f) {
	State::RESET();
	Symbol::RESET();
	Weight::RESET();

	std::string name = "LiveOf(" + A->getName() + ")";

	MapArray<Symbol*>* newalphabet = new MapArray<Symbol*>(A->alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < A->alphabet->size(); ++symbol_id) {
		newalphabet->insert(symbol_id, new Symbol(A->alphabet->at(symbol_id)));
	}

	MapArray<State*>* newstates = new MapArray<State*>(A->states->size());
	for (unsigned int state_id = 0; state_id < A->states->size(); ++state_id) {
		newstates->insert(state_id, new State(A->states->at(state_id)));
	}
	State* newinitial = newstates->at(A->initial->getId());

	weight_t top_values[A->nb_SCCs];
	A->compute_Top(f, top_values);

	SetSorted<weight_t> weight_values;
	weight_values.insert(top_values[A->initial->getTag()]);
	for (unsigned int weight_id = 0; weight_id < A->weights->size(); ++weight_id) {
		weight_values.insert(A->weights->at(weight_id)->getValue());
	}
	weight_t newmin_domain = A->min_domain;//domain does not change
	weight_t newmax_domain = A->max_domain;//domain does not depend on weights

	MapStd<weight_t, Weight*> weight_register;
	MapArray<Weight*>* newweights = new MapArray<Weight*>(weight_values.size());
	for (weight_t value : weight_values) {
		Weight* weight = new Weight(value);
		newweights->insert(weight->getId(), weight);
		weight_register.insert(weight->getValue(), weight);
	}

	for (unsigned int state_id = 0; state_id < A->states->size(); ++state_id) {
		for (Symbol* symbol : *(A->states->at(state_id)->getAlphabet())) {
			for (Edge* edge : *(A->states->at(state_id)->getSuccessors(symbol->getId()))) {
				State* from = newstates->at(edge->getFrom()->getId());
				State* to = newstates->at(edge->getTo()->getId());
				weight_t value;
				if (edge->getWeight()->getValue() == top_values[from->getTag()]) {
					value = top_values[A->initial->getTag()];
				}
				else {
					value = edge->getWeight()->getValue();
				}
				Weight* weight = weight_register.at(value);
				Edge* new_edge = new Edge(newalphabet->at(symbol->getId()), weight, from, to);
				from->addSuccessor(new_edge);
				to->addPredecessor(new_edge);
			}
		}
	}

	return new Automaton(name, newalphabet, newstates, newweights, newmin_domain, newmax_domain, newinitial);
}



void explore_monotonically (
		std::pair<State*, Weight*> from,
		SetStd<std::pair<State*, Weight*>> set_of_states,
		SetStd<std::pair<Symbol*, std::pair<std::pair<State*, Weight*>, std::pair<State*, Weight*>>>> set_of_edges,
		Weight* (*select_weight)(Weight*, Weight*)
) {
	set_of_states.insert(from);

	for (Symbol* symbol : *((from.first)->getAlphabet())) {
		for (Edge* edge : *((from.first)->getSuccessors(symbol->getId()))) {
			State* state = edge->getTo();
			Weight* weight = select_weight(from.second, edge->getWeight());
			auto to = std::pair<State*, Weight*>(state, weight);
			auto pair_of_states = std::pair<std::pair<State*, Weight*>, std::pair<State*, Weight*>>(from, to);
			auto newedge = std::pair<Symbol*, std::pair<std::pair<State*, Weight*>, std::pair<State*, Weight*>>>(symbol, pair_of_states);
			set_of_edges.insert(newedge);
			if (set_of_states.contains(to) == false) {
				explore_monotonically(to, set_of_states, set_of_edges, select_weight);
			}
		}
	}
}


void explore_Inf (
		std::pair<State*, Weight*> from,
		SetStd<std::pair<State*, Weight*>> set_of_states,
		SetStd<std::pair<Symbol*, std::pair<std::pair<State*, Weight*>, std::pair<State*, Weight*>>>> set_of_edges
){
	Weight* (*select_weight)(Weight*, Weight*);
	select_weight = [] (Weight* x, Weight* y) -> Weight* {
		return ((x->getValue() < y->getValue()) ? x : y);
	};
	explore_monotonically(from, set_of_states, set_of_edges, select_weight);
}


void explore_Sup (
		std::pair<State*, Weight*> from,
		SetStd<std::pair<State*, Weight*>> set_of_states,
		SetStd<std::pair<Symbol*, std::pair<std::pair<State*, Weight*>, std::pair<State*, Weight*>>>> set_of_edges
){
	Weight* (*select_weight)(Weight*, Weight*);
	select_weight = [] (Weight* x, Weight* y) -> Weight* {
		return ((x->getValue() < y->getValue()) ? y : x);
	};
	explore_monotonically(from, set_of_states, set_of_edges, select_weight);
}


void explore_LimInf (
		std::pair<State*, Weight*> from,
		SetStd<std::pair<State*, Weight*>> set_of_states,
		SetStd<std::pair<Symbol*, std::pair<std::pair<State*, Weight*>, std::pair<State*, Weight*>>>> set_of_edges
) {
	set_of_states.insert(from);

	for (Symbol* symbol : *((from.first)->getAlphabet())) {
		for (Edge* edge : *((from.first)->getSuccessors(symbol->getId()))) {
			if (edge->getWeight()->getValue() >= from.second->getValue()) {
				State* state = edge->getTo();
				auto to1 = std::pair<State*, Weight*>(state, from.second);
				auto pair1 = std::pair<std::pair<State*, Weight*>, std::pair<State*, Weight*>>(from, to1);
				auto edge1 = std::pair<Symbol*, std::pair<std::pair<State*, Weight*>, std::pair<State*, Weight*>>>(symbol, pair1);
				set_of_edges.insert(edge1);
				if (set_of_states.contains(to1) == false) {
					explore_LimInf(to1, set_of_states, set_of_edges);
				}
				auto to2 = std::pair<State*, Weight*>(state, edge->getWeight());
				auto pair2 = std::pair<std::pair<State*, Weight*>, std::pair<State*, Weight*>>(from, to2);
				auto edge2 = std::pair<Symbol*, std::pair<std::pair<State*, Weight*>, std::pair<State*, Weight*>>>(symbol, pair2);
				set_of_edges.insert(edge2);
				if (set_of_states.contains(to2) == false) {
					explore_LimInf(to2, set_of_states, set_of_edges);
				}
			}
		}
	}
}


Automaton* Automaton::toLimSup (const Automaton* A, value_function_t f) {
	State::RESET();
	Symbol::RESET();
	Weight::RESET();

	void (*explore)(
			std::pair<State*, Weight*> from,
			SetStd<std::pair<State*, Weight*>> set_of_states,
			SetStd<std::pair<Symbol*, std::pair<std::pair<State*, Weight*>, std::pair<State*, Weight*>>>> set_of_edges
	);
	switch(f) {
	case Inf:
		explore = explore_Inf;
		break;
	case Sup:
		explore = explore_Sup;
		break;
	case LimInf:
		explore = explore_LimInf;
		break;
	case LimSup: case LimAvg:
		fail("invalid translation to LimSup");
	default:
		fail("invalid value function");
	}

	std::string newname = "LimSup(" + A->getName() + ")";

	MapArray<Symbol*>* newalphabet = new MapArray<Symbol*>(A->alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < A->alphabet->size(); ++symbol_id) {
		newalphabet->insert(symbol_id, new Symbol(A->alphabet->at(symbol_id)));
	}

	MapArray<Weight*>* newweights = new MapArray<Weight*>(A->weights->size());
	for (unsigned int weight_id = 0; weight_id < A->weights->size(); ++weight_id) {
		newweights->insert(weight_id, new Weight(A->weights->at(weight_id)));
	}

	weight_t newmin_domain = A->min_domain;//domain does not change
	weight_t newmax_domain = A->max_domain;//domain does not depend on weights

	SetStd<std::pair<State*, Weight*>> set_of_states;
	SetStd<std::pair<Symbol*, std::pair<std::pair<State*, Weight*>, std::pair<State*, Weight*>>>> set_of_edges;
	auto start = std::pair<State*, Weight*>(A->initial, A->weights->at(0));
	explore(start, set_of_states, set_of_edges);

	MapArray<State*>* newstates = new MapArray<State*>(A->alphabet->size());
	MapStd<std::pair<State*, Weight*>, State*> state_register;
	for (std::pair<State*, Weight*> pair : set_of_states) {
		std::string statename = "(" + pair.first->getName() + ", " + std::to_string(pair.second->getValue());
		State* state = new State(statename, newalphabet->size(), newmin_domain, newmax_domain);
		newstates->insert(state->getId(), state);
		state_register.insert(pair, state);
	}
	State* newinitial = state_register.at(start);

	for (auto pair : set_of_edges) {
		Symbol* symbol = newalphabet->at(pair.first->getId());
		Weight* weight = newweights->at(pair.second.second.second->getId());
		State* from = state_register.at(pair.second.first);
		State* to = state_register.at(pair.second.second);
		Edge *edge = new Edge(symbol, weight, from, to);
		from->addSuccessor(edge);
		to->addPredecessor(edge);
	}

	return new Automaton(newname, newalphabet, newstates, newweights, newmin_domain, newmax_domain, newinitial);
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

bool Automaton::isEmpty (value_function_t f, weight_t x ) const {
	return (getTopValue(f) >=x);
}

bool Automaton::isUniversal (value_function_t f, weight_t x) const {
	return (getBottomValue(f) >= x);
}




bool Automaton::isLimAvgConstant() const {
	weight_t top = getTopValue(LimAvg);

    int dist[this->getStates()->size()];
    for (unsigned int state_id = 0; state_id < this->getStates()->size(); ++state_id) {
        dist[state_id] = 0;
    }

    for (unsigned int state_id = 0; state_id < this->getStates()->size(); ++state_id) {
        for (Symbol* symbol : *(this->getStates()->at(state_id)->getAlphabet())) {
        	for (Edge* edge : *(this->getStates()->at(state_id)->getSuccessors(symbol->getId()))) {
    			unsigned int u = edge->getFrom()->getId();
				unsigned int v = edge->getTo()->getId();
				//weights are inversed AND shifted by 'top': -(edge-top) = top-edge
				weight_t value = top - edge->getWeight()->getValue();
				if (dist[u] + value < dist[v]) {
					dist[v] = dist[u] + value;
				}
        	}
        }
    }

	State::RESET();
	Symbol::RESET();
	Weight::RESET();

	std::string newname = "Lol(" + this->getName() + ")";

	MapArray<Symbol*>* newalphabet = new MapArray<Symbol*>(this->alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
		newalphabet->insert(symbol_id, new Symbol(this->alphabet->at(symbol_id)));
	}

	MapArray<State*>* newstates = new MapArray<State*>(this->states->size());
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		newstates->insert(state_id, new State(this->states->at(state_id)));
	}
	State* newinitial = newstates->at(this->initial->getId());

	MapArray<Weight*>* newweights = new MapArray<Weight*>(2);
	for (unsigned int weight_id = 0; weight_id < 2; ++weight_id) {
		newweights->insert(weight_id, new Weight(weight_id));
	}

	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		for (Symbol* symbol : *(this->states->at(state_id)->getAlphabet())) {
			for (Edge* edge : *(this->states->at(state_id)->getSuccessors(symbol->getId()))) {
				unsigned int u = edge->getFrom()->getId();
				unsigned int v = edge->getTo()->getId();
				//originally: -(edge-top) + from - to = 0
				//inverted weights: (edge-top) - from + to = 0
				//equivalently: edge - from + to = top
				weight_t value = (((edge->getWeight()->getValue() - dist[u] + dist[v]) == top) ? 1 : 0);
				Weight* weight = newweights->at(value);
				State* from = newstates->at(edge->getFrom()->getId());
				State* to = newstates->at(edge->getTo()->getId());
				Edge* newedge = new Edge(newalphabet->at(symbol->getId()), weight, from, to);
				newstates->at(state_id)->addSuccessor(newedge);
				newstates->at(edge->getTo()->getId())->addPredecessor(newedge);
			}
		}
	}

	Automaton* LOL = new Automaton(newname, newalphabet, newstates, newweights, 0, 1, newinitial);
	bool out = isUniversal(LimInf, 1);
	delete LOL;
	return out;
}


// Remark: If deterministic LimAvg then 'isDeterministic' is called twice
bool Automaton::isConstant (value_function_t f) const {
	if (f == LimAvg && isDeterministic() == false) {
		return isLimAvgConstant();
	}
	else {
		return (getTopValue(f) == getBottomValue(f));
	}
}


bool Automaton::isIncludedIn(const Automaton* B, value_function_t f) const {
	if (f == LimAvg) {
		if (B->isDeterministic()) {
			Automaton* C = Automaton::product(this, Minus, B);
			weight_t Ctop = C->getTopValue(f);
			delete C;
			return (Ctop < 0);
		}
		else {
			fail("automata inclusion undecidable for nondeterministic limavg");
		}
	}
	else if (f == Inf || f == Sup || f == LimInf || f == LimSup) {
		bool flag;

		if (f == LimSup) {
			flag = inclusion(this, B);
		}
		else {
			Automaton* AA = Automaton::toLimSup(this, f);
			Automaton* BB = Automaton::toLimSup(B, f);
			flag = inclusion(AA, BB);
			delete AA;
			delete BB;
		}

		return flag;
	}
	else {
		fail("automata inclusion type");
	}
}


bool Automaton::isSafe (value_function_t f) const {
	Automaton* S = Automaton::safetyClosure(this, f);
	bool out = S->isIncludedIn(this, f);
	delete S;
	return out;
}

bool Automaton::isLive (value_function_t f) const {
	Automaton* S = Automaton::safetyClosure(this, f);
	Automaton* C = Automaton::product(this, Minus, S);
	bool out = C->isConstant(f);
	delete S;
	delete C;
	return out;
}










// -------------------------------- Tops -------------------------------- //


void Automaton::top_reachably_scc (State* state, bool in_scc, bool* spot, weight_t* values) const {
	if (spot[state->getId()] == true) return;
	spot[state->getId()] = true;
	values[state->getId()] = this->min_domain;
	for (Symbol* symbol : *(state->getAlphabet())) {
		for (Edge* edge : *(state->getSuccessors(symbol->getId()))) {
			if (edge->getTo()->getTag() == state->getTag()) {
				top_reachably_scc(edge->getTo(), in_scc, spot, values);
				values[state->getId()] = std::max(values[state->getId()], edge->getWeight()->getValue());
				values[state->getId()] = std::max(values[state->getId()], values[edge->getTo()->getId()]);
			}
			else if (in_scc == true) {
				values[state->getId()] = std::max(values[state->getId()], edge->getWeight()->getValue());
			}
		}
	}
}


void Automaton::top_reachably_tree (SCC_Tree* tree, bool in_scc, bool* spot, weight_t* values, weight_t top_values[]) const {
	top_reachably_scc(tree->origin, in_scc, spot, values);
	top_values[tree->origin->getTag()] = values[tree->origin->getId()];

	for (auto iter = tree->nexts->begin(); iter != tree->nexts->end(); ++iter) {
		top_reachably_tree(*iter, in_scc, spot, values, top_values);
		top_values[tree->origin->getTag()] = std::max(top_values[tree->origin->getTag()],
				top_values[(*iter)->origin->getTag()]);
	}
}


weight_t Automaton::top_Sup (weight_t* top_values) const {
	weight_t values[this->states->size()];
	bool spot[this->states->size()];

	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		spot[state_id] = false;
	}

	top_reachably_tree(this->SCCs_tree, false, spot, values, top_values);
	return top_values[this->SCCs_tree->origin->getTag()];
}


weight_t Automaton::top_LimSup (weight_t* top_values) const {
	weight_t values[this->states->size()];
	bool spot[this->states->size()];

	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		spot[state_id] = false;
	}

	top_reachably_tree(this->SCCs_tree, true, spot, values, top_values);
	return top_values[this->SCCs_tree->origin->getTag()];
}


void Automaton::top_safety_scc_recursive (Edge* edge, bool in_scc, int* values, int** counters) const {
	State* state = edge->getFrom();
	if (counters[state->getId()][edge->getSymbol()->getId()] == 0) return;

	counters[state->getId()][edge->getSymbol()->getId()]--;
	if (counters[state->getId()][edge->getSymbol()->getId()] == 0) {
		weight_t max_value = this->min_domain;
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
	MapArray<SetList<Edge*>*> edges(this->weights->size());
	for (unsigned int weight_id = 0; weight_id < this->weights->size(); ++weight_id) {
		edges.insert(weight_id, new SetList<Edge*>);
	}

	//O(m+n)
	int* counters[this->states->size()];
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		counters[state_id] = new int[this->alphabet->size()];
		values[state_id] = this->max_domain;
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
		top_values[scc_id] = this->min_domain;
	}

	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		if (values[state_id] <= this->max_domain) {
			top_values[this->states->at(state_id)->getTag()] =
					std::max(top_values[this->states->at(state_id)->getTag()], values[state_id]);
		}

	}

	top_safety_tree(this->SCCs_tree, top_values);
	return top_values[this->SCCs_tree->origin->getTag()];
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
	weight_t infinity = -(size*(this->min_domain)) - 1;

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
								distance[len][edge->getTo()->getId()] =
										std::min(value, distance[len][edge->getTo()->getId()]);
							}
						}
					}
				}
			}
		}
	}

	//O(n.m)
	for (unsigned int scc_id = 0; scc_id < this->nb_SCCs; ++scc_id) {
		top_values[scc_id] = this->min_domain;
	}

	for (unsigned int state_id = 0; state_id < size; ++state_id) {
		weight_t min_lenght_avg = this->max_domain;
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
	return top_values[this->SCCs_tree->origin->getTag()];
}

weight_t Automaton::compute_Top (value_function_t f, weight_t* top_values) const {
	switch (f) {
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


weight_t Automaton::compute_Bottom (value_function_t f, weight_t* bot_values) const {
	if (this->isDeterministic()) {
		invert_weights();
		weight_t bot = compute_Top(f, bot_values);
		bot = -bot;
		for (unsigned int i = 0; i < this->nb_SCCs; i++) {
			bot_values[i] = -bot_values[i];
		}
		invert_weights();
		return bot;
	}
	else {
		if (f == Inf || f == Sup || f == LimInf || f == LimSup) {
			bool found = false;
			unsigned int weight_id = weights->size();
			weight_t x;

			while (!found && weight_id > 0) {
				weight_id--;
				x = this->weights->at(weight_id)->getValue();

				Automaton* C = Automaton::constantAutomaton(this, x);
				found = this->isIncludedIn(C, f);
				delete C;
			}

			return x;
		}
		else {
			fail("automata bot");
		}
	}
}









// -------------------------------- toStrings -------------------------------- //


void Automaton::print () const {
	std::cout << "automaton (" << this->name << "):\n";
	std::cout << "\talphabet (" << this->alphabet->size() << "):";
	std::cout << this->alphabet->toString(Symbol::toString) << "\n";
	std::cout << "\tweights (" << this->weights->size() << "):";
	std::cout << weights->toString(Weight::toString) << "\n";
	std::cout << "\t\tMIN = " << std::to_string(min_domain) << "\n";
	std::cout << "\t\tMAX = " << std::to_string(max_domain) << "\n";
	std::cout << "\tstates (" << this->states->size() << "):";
	std::cout << states->toString(State::toString) << "\n";
	std::cout << "\t\tINITIAL = " << initial->getName() << "\n";
	/*std::cout << "\tSCCs (" << this->nb_SCCs << "):";
	std::cout << this->SCCs_tree->toString("\t\t") << "\n";
	unsigned int nb_edge = 0;
	for (unsigned int state_id = 0; state_id < states->size(); ++state_id) {
		for (Symbol* symbol : *(states->at(state_id)->getAlphabet())) {
			nb_edge += states->at(state_id)->getSuccessors(symbol->getId())->size();
		}
	}
	std::cout << "\tedges (" << nb_edge << "):";
	for (unsigned int state_id = 0; state_id < states->size(); ++state_id) {
		for (Symbol* symbol : *(states->at(state_id)->getAlphabet())) {
			std::cout << states->at(state_id)->getSuccessors(symbol->getId())->toString(Edge::toString);
		}
	}
	std::cout << "\n";*/
}





// -------------------------------- Nicolas -------------------------------- //





