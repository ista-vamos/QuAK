#include <set>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

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

weight_t aggregator_apply (aggregator_t aggregator, weight_t x, weight_t y) {
	switch (aggregator) {
		case Max: return std::max(x, y);
		case Min: return std::min(x, y);
		case Plus: return x + y;
		case Minus: return x - y;
		case Times: return x * y;
		default: fail("case aggregator_t");
	}
}


Automaton* Automaton::product(const Automaton* A, aggregator_t f, const Automaton* B) {
	MapStd<std::string, Symbol*> sync_register;
	Parser parser(aggregator_apply(f, A->min_domain, B->min_domain), aggregator_apply(f, A->max_domain, B->max_domain));

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
						weight_t weightvalue = aggregator_apply(f, edgeA->getWeight()->getValue(), edgeB->getWeight()->getValue());
						std::string fromname =  "(" + edgeA->getFrom()->getName() + "," + edgeB->getFrom()->getName() + ")";
						std::string toname =  "(" + edgeA->getTo()->getName() + "," + edgeB->getTo()->getName() + ")";

						parser.alphabet.insert(symbolname);
						parser.weights.insert(weightvalue);
						parser.states.insert(fromname);
						parser.states.insert(toname);

						if (A->initial->getId() == stateA_id && B->initial->getId() == stateB_id) {
							parser.initial = fromname;
						}

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

	bool sinkFlag = false;
	for (unsigned int stateA_id = 0; stateA_id < A->getStates()->size(); ++stateA_id) {
		if (A->getStates()->at(stateA_id)->getTag() == -1) continue;
		for (unsigned int symbol_id = 0; symbol_id < A->getAlphabet()->size(); ++symbol_id) {
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
			sinkFlag = true;
		}
	}

	if (sinkFlag) {
		for (std::string symbolname: parser->alphabet) {
			std::pair<std::pair<std::string, weight_t>,std::pair<std::string, std::string>> edge;
			edge.first.first = symbolname;
			edge.first.second = sinkvalue;
			edge.second.first = "#sink#";
			edge.second.second = "#sink#";
			parser->edges.insert(edge);
		}
	}

	parser->initial = A->getInitial()->getName();

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
	weight_t top = compute_Top(f, top_values);
	/*for (unsigned int id = 0;id <this->nb_SCCs; id++) {
		printf("top[%u] = %s\n", id, std::to_string(top_values[id]).c_str());
	}*/
	return top;
}

weight_t Automaton::getBottomValue (value_function_t f) {
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



void Automaton::invert_weights() {
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

	weight_t temp = this->max_domain;
	this->setMaxDomain(-this->getMinDomain());
	this->setMinDomain(-temp);
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
		if (A->states->at(state_id)->getTag() == -1) continue;
		for (Symbol* symbol : *(A->states->at(state_id)->getAlphabet())) {
			for (Edge* edge : *(A->states->at(state_id)->getSuccessors(symbol->getId()))) {
				State* from = newstates->at(edge->getFrom()->getId());
				State* to = newstates->at(edge->getTo()->getId());
				Weight* weight = weight_register.at(top_values[edge->getTo()->getTag()]);
				Edge* newedge = new Edge(newalphabet->at(symbol->getId()), weight, from, to);
				from->addSuccessor(newedge);
				to->addPredecessor(newedge);
			}
		}
	}

	return new Automaton(newname, newalphabet, newstates, newweights, newmin_domain, newmax_domain, newinitial);
}



Automaton* Automaton::livenessComponent_deterministic (const Automaton* A, value_function_t f) {
	if (!((f == Sup || f == LimInf || f == LimSup) && A->isDeterministic())) {
		fail("invalid automaton type for liveness component (deterministic)");
	}

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



// TODO
Automaton* Automaton::livenessComponent_prefixIndependent (const Automaton* A, value_function_t f) {
	if (!(f == LimInf || f == LimSup || f == LimAvg)) {
		fail("invalid automaton type for liveness component (prefix independent)");
	}

	State::RESET();
	Symbol::RESET();
	Weight::RESET();

	// std::string name = "LiveOf(" + A->getName() + ")";

	// MapArray<Symbol*>* newalphabet = new MapArray<Symbol*>(A->alphabet->size());
	// for (unsigned int symbol_id = 0; symbol_id < A->alphabet->size(); ++symbol_id) {
	// 	newalphabet->insert(symbol_id, new Symbol(A->alphabet->at(symbol_id)));
	// }

	
	// MapArray<State*>* newstates = new MapArray<State*>(A->states->size());
	// for (unsigned int state_id = 0; state_id < A->states->size(); ++state_id) {
	// 	newstates->insert(state_id, new State(A->states->at(state_id)));
	// }
	// State* newinitial = newstates->at(A->initial->getId());

	// weight_t top_values[A->nb_SCCs];
	// A->compute_Top(f, top_values);

	// SetStd<weight_t> weight_values;
	// weight_values.insert(top_values[A->initial->getTag()]);
	// for (unsigned int weight_id = 0; weight_id < A->weights->size(); ++weight_id) {
	// 	weight_values.insert(A->weights->at(weight_id)->getValue());
	// }
	// weight_t newmin_domain = A->min_domain;//domain does not change
	// weight_t newmax_domain = A->max_domain;//domain does not depend on weights

	// MapStd<weight_t, Weight*> weight_register;
	// MapArray<Weight*>* newweights = new MapArray<Weight*>(weight_values.size());
	// for (weight_t value : weight_values) {
	// 	Weight* weight = new Weight(value);
	// 	newweights->insert(weight->getId(), weight);
	// 	weight_register.insert(weight->getValue(), weight);
	// }

	// for (unsigned int state_id = 0; state_id < A->states->size(); ++state_id) {
	// 	for (Symbol* symbol : *(A->states->at(state_id)->getAlphabet())) {
	// 		for (Edge* edge : *(A->states->at(state_id)->getSuccessors(symbol->getId()))) {
	// 			State* from = newstates->at(edge->getFrom()->getId());
	// 			State* to = newstates->at(edge->getTo()->getId());
	// 			weight_t value;
	// 			if (edge->getWeight()->getValue() == top_values[from->getTag()]) {
	// 				value = top_values[A->initial->getTag()];
	// 			}
	// 			else {
	// 				value = edge->getWeight()->getValue();
	// 			}
	// 			Weight* weight = weight_register.at(value);
	// 			Edge* new_edge = new Edge(newalphabet->at(symbol->getId()), weight, from, to);
	// 			from->addSuccessor(new_edge);
	// 			to->addPredecessor(new_edge);
	// 		}
	// 	}
	// }

	// return new Automaton(name, newalphabet, newstates, newweights, newmin_domain, newmax_domain, newinitial);
}


void explore_monotonically (
		std::pair<State*, Weight*> &from,
		SetStd<std::pair<State*, Weight*>> &set_of_states,
		SetStd<std::pair<Symbol*, std::pair<std::pair<State*, Weight*>, std::pair<State*, Weight*>>>> &set_of_edges,
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
		std::pair<State*, Weight*> &from,
		SetStd<std::pair<State*, Weight*>> &set_of_states,
		SetStd<std::pair<Symbol*, std::pair<std::pair<State*, Weight*>, std::pair<State*, Weight*>>>> &set_of_edges
){
	Weight* (*select_weight)(Weight*, Weight*);
	select_weight = [] (Weight* x, Weight* y) -> Weight* {
		return ((x->getValue() < y->getValue()) ? x : y);
	};
	explore_monotonically(from, set_of_states, set_of_edges, select_weight);
}


void explore_Sup (
		std::pair<State*, Weight*> &from,
		SetStd<std::pair<State*, Weight*>> &set_of_states,
		SetStd<std::pair<Symbol*, std::pair<std::pair<State*, Weight*>, std::pair<State*, Weight*>>>> &set_of_edges
){
	Weight* (*select_weight)(Weight*, Weight*);
	select_weight = [] (Weight* x, Weight* y) -> Weight* {
		return ((x->getValue() > y->getValue()) ? y : x);
	};
	explore_monotonically(from, set_of_states, set_of_edges, select_weight);
}


void explore_LimInf (
		std::pair<State*, Weight*> &from,
		SetStd<std::pair<State*, Weight*>> &set_of_states,
		SetStd<std::pair<Symbol*, std::pair<std::pair<State*, Weight*>, std::pair<State*, Weight*>>>> &set_of_edges
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

  using std::pair;

	State::RESET();
	Symbol::RESET();
	Weight::RESET();

	int initWeightId;
	void (*explore)(
			pair<State*, Weight*> &from,
			SetStd<pair<State*, Weight*>> &set_of_states,
			SetStd<pair<Symbol*, pair<pair<State*, Weight*>, pair<State*, Weight*>>>> &set_of_edges
	);
	switch(f) {
	case Inf:
		explore = explore_Inf;
		initWeightId = A->getWeights()->size() - 1;
		break;
	case Sup:
		explore = explore_Sup;
		initWeightId = 0;
		break;
	case LimInf:
		explore = explore_LimInf;
		initWeightId = 0;
		break;
	case LimSup: case LimAvg:
		fail("invalid translation to LimSup");
	default:
		fail("invalid value function");
	}

	std::string newname = "LimSup(" + A->getName() + ")";

  // copy the alphabet
	MapArray<Symbol*>* newalphabet = new MapArray<Symbol*>(A->alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < A->alphabet->size(); ++symbol_id) {
		newalphabet->insert(symbol_id, new Symbol(A->alphabet->at(symbol_id)));
	}

	// TODO: should we adjust the domain with the new weights?
	// definitely not!
	// the domain should only be enlarged
	// an automaton may have only weight 1, but domain 0,1
	// if the domain gets reduced to 1,1 the automaton becomes constant
	weight_t newmin_domain = A->min_domain;//domain does not change
	weight_t newmax_domain = A->max_domain;//domain does not depend on weights

	SetStd<pair<State*, Weight*>> set_of_states;
	SetStd<pair<Symbol*, pair<pair<State*, Weight*>, pair<State*, Weight*>>>> set_of_edges;
	auto start = pair<State*, Weight*>(A->initial, A->weights->at(initWeightId));
	explore(start, set_of_states, set_of_edges);

	// TODO: it may be that some weights are not seen in the limsup automaton
	// MapArray<Weight*>* newweights = new MapArray<Weight*>(A->weights->size());
	// for (unsigned int weight_id = 0; weight_id < A->weights->size(); ++weight_id) {
	// 	newweights->insert(weight_id, new Weight(A->weights->at(weight_id)));
	// }
	
	// collect the weights that actually occur in the new automaton
	std::set<unsigned int> tempWeightIds;
	for (const auto &edge : set_of_edges) {
		tempWeightIds.insert(edge.second.second.second->getId());
	}
	MapArray<Weight*>* newweights = new MapArray<Weight*>(tempWeightIds.size());
	int ctr = 0;
	for (auto weight_id : tempWeightIds) {
		newweights->insert(ctr, new Weight(A->weights->at(weight_id)));
		ctr++;
	}

	MapArray<State*>* newstates = new MapArray<State*>(set_of_states.size());
	MapStd<pair<State*, Weight*>, State*> state_register;
	for (const pair<State*, Weight*> &weighted_state : set_of_states) {
		std::string statename = "(" + weighted_state.first->getName() + ", " + std::to_string(weighted_state.second->getValue()) + ")";
		State* state = new State(statename, newalphabet->size(), newmin_domain, newmax_domain);
		newstates->insert(state->getId(), state);
		state_register.insert(weighted_state, state);
	}
	State* newinitial = state_register.at(start);

	for (const auto &edgeA : set_of_edges) {
		Symbol* symbol = newalphabet->at(edgeA.first->getId());
		Weight* weight = newweights->at(edgeA.second.second.second->getId());
		State* from = state_register.at(edgeA.second.first);
		State* to = state_register.at(edgeA.second.second);
		Edge *edge = new Edge(symbol, weight, from, to);
		from->addSuccessor(edge);
		to->addPredecessor(edge);
	}

	return new Automaton(newname, newalphabet, newstates, newweights, newmin_domain, newmax_domain, newinitial);
}

std::vector<unsigned int> int2function (unsigned int id, unsigned int n, unsigned int m) {
	std::vector<unsigned int> func(n, 0);
	for (int i = n - 1; i >= 0; i--) {
		func[i] = id % m;
		id = id / m;
	}
	return func;
}

unsigned int function2int (std::vector<unsigned int> func, unsigned int n, unsigned int m) {
	unsigned int id = 0;
	unsigned int b = 1;
	for (int i = n - 1; i >= 0; i--) {
		id = id + func[i] * b;
		b = b * m;
	}
	return id;
}


// TODO: finish
Automaton* Automaton::determinizeInf (const Automaton* A) {
	State::RESET();
	Symbol::RESET();
	Weight::RESET();
	
	MapArray<Symbol*>* newalphabet = new MapArray<Symbol*>(A->alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < A->alphabet->size(); ++symbol_id) {
		newalphabet->insert(symbol_id, new Symbol(A->alphabet->at(symbol_id)));
	}

	std::unordered_map<weight_t, unsigned int> weightInverseMap;
	MapArray<Weight*>* newweights = new MapArray<Weight*>(A->weights->size());
	for (unsigned int weight_id = 0; weight_id < A->weights->size(); ++weight_id) {
		newweights->insert(weight_id, new Weight(A->weights->at(weight_id)));
		weightInverseMap[A->weights->at(weight_id)->getValue()] = weight_id;
	}

	weight_t newmin_domain = A->min_domain;
	weight_t newmax_domain = A->max_domain;

	unsigned int n = A->states->size();
	unsigned int m = A->weights->size();

	unsigned int size = 1;
	for (unsigned int state_id = 0; state_id < n; ++state_id) {
		size = size * m;
	}

	MapArray<State*>* newstates = new MapArray<State*>(size); // each state represents a function from states of A to weightIds of A
	for (unsigned int state_id = 0; state_id < size; ++state_id) {
		std::string statename = "(";
		std::vector<unsigned int> funcTemp = int2function(state_id, n, m);
		for (unsigned int i = 0; i < n - 1; i++) {
			statename += std::to_string(funcTemp[i]) + ",";
		}
		statename += std::to_string(funcTemp[n-1]) + ")";
		State* state = new State(statename, newalphabet->size(), newmin_domain, newmax_domain);
		newstates->insert(state->getId(), state);
	}

	weight_t top_values[A->nb_SCCs];
	std::vector<unsigned int> funcFrom(n, 0);
	funcFrom[A->getInitial()->getId()] = weightInverseMap[A->compute_Top(Inf, top_values)];
	unsigned int initIndex = function2int(funcFrom, n, m);
	State* newinitial = newstates->at(initIndex);

	for (unsigned int state_id = 0; state_id < size; ++state_id) {
		funcFrom = int2function(state_id, n, m);

		for (unsigned int symbol_id = 0; symbol_id < A->alphabet->size(); ++symbol_id) {
			std::vector<unsigned int> funcTo(n, 0);
			for (unsigned int to_id = 0; to_id < n; to_id++) {
				for (Edge* edge : *(A->states->at(to_id)->getPredecessors(symbol_id))) {
					unsigned int from_id = edge->getFrom()->getId();
					unsigned int x = funcFrom[from_id];
					unsigned int y = edge->getWeight()->getId();
					unsigned int z = funcTo[to_id];
					funcTo[to_id] = std::max(z, std::min(x, y)); // this is fine because weights are ordered
				}
			}

			unsigned int new_to_id = function2int(funcTo, n, m);
			State* from = newstates->at(state_id);
			State* to = newstates->at(new_to_id);

			unsigned int weight_id = 0;
			for (unsigned int i = 0; i < n; i++) {
				weight_id = std::max(weight_id, funcFrom[i]);
			}
			
			Weight* w = newweights->at(weight_id);
			Edge* newedge = new Edge(newalphabet->at(symbol_id), w, from, to);
			from->addSuccessor(newedge);
			to->addPredecessor(newedge);
		}
	}


	std::string newname = "Determinized(" + A->getName() + ")";
	Automaton* that = new Automaton(newname, newalphabet, newstates, newweights, newmin_domain, newmax_domain, newinitial);
	Automaton* AA = copy_trim_complete(that, Inf);
	delete that;
	return AA;


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

bool Automaton::isEmpty (value_function_t f, weight_t x ) {
	return (getTopValue(f) >=x);
}

bool Automaton::isUniversal (value_function_t f, weight_t x)  {
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

	std::string newname = "Dist(" + this->getName() + ")";

	MapArray<Symbol*>* newalphabet = new MapArray<Symbol*>(this->alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
		newalphabet->insert(symbol_id, new Symbol(this->alphabet->at(symbol_id)));
	}

	MapArray<State*>* newstates = new MapArray<State*>(this->states->size());
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		newstates->insert(state_id, new State(this->states->at(state_id)));
	}
	State* newinitial = newstates->at(this->initial->getId());

	bool weightsSeen[2];
	weightsSeen[0] = false;
	weightsSeen[1] = false;
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		for (Symbol* symbol : *(this->states->at(state_id)->getAlphabet())) {
			for (Edge* edge : *(this->states->at(state_id)->getSuccessors(symbol->getId()))) {
				unsigned int u = edge->getFrom()->getId();
				unsigned int v = edge->getTo()->getId();
				if ((edge->getWeight()->getValue() - dist[u] + dist[v]) == top) {
					weightsSeen[1] = true;
				}
				else {
					weightsSeen[0] = true;
				}
			}
		}
	}

	if (weightsSeen[0] && !weightsSeen[1]) {
		return false;
	}
	if (!weightsSeen[0] && weightsSeen[1]) {
		return true; 
	}

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

	Automaton* Dist = new Automaton(newname, newalphabet, newstates, newweights, 0, 1, newinitial);
	bool out = Dist->isUniversal(LimInf, 1);
	// Dist->print();
	delete Dist;
	return out;
}


bool Automaton::isConstant (value_function_t f) {
	if (f == LimAvg && isDeterministic() == false) {
		return isLimAvgConstant();
	}
	else {
		std::cout << getTopValue(f) << " " << getBottomValue(f) << std::endl;
		return (getTopValue(f) == getBottomValue(f));
	}
}

bool Automaton::isIncludedIn(const Automaton* B, value_function_t f, bool booleanized) {
    if (booleanized) {
        return isIncludedIn_booleanized(B, f);
    }

    return isIncludedIn_antichains(B, f);
}

bool Automaton::isIncludedIn_antichains(const Automaton* B, value_function_t f) {
	if (f == LimAvg) {
		if (B->isDeterministic()) {
			Automaton* C = Automaton::product(this, Minus, B);
			C->print();
			weight_t Ctop = C->getTopValue(f);
			delete C;
			return (Ctop <= 0);
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

bool Automaton::isIncludedIn_booleanized(const Automaton* B, value_function_t f) {
    for (auto *weight: *weights) {
        auto boolA = std::unique_ptr<Automaton>(booleanize(this, weight->getValue()));
        auto boolB = std::unique_ptr<Automaton>(booleanize(B, weight->getValue()));

        if (!boolA->isIncludedIn_antichains(boolB.get(), f))
            return false;
    }

    return true;
}


bool Automaton::isSafe (value_function_t f) {
/*	if (f == Inf) {
		return true;
	}
*/
	Automaton* S = Automaton::safetyClosure(this, f);
	bool out;

	if (f == LimAvg && !this->isDeterministic()) {
		Automaton* SS = Automaton::determinizeInf(S);
		Automaton* C = Automaton::product(this, Minus, SS);
		out = C->isLimAvgConstant();
		delete SS;
		delete C;
	}
	else {
		out = S->isIncludedIn(this, f);
	}

	delete S;
	return out;
}

bool Automaton::isLive (value_function_t f) {
	bool out;
/*
	if (f == Inf) {
		return this->isConstant(Inf);
	}
*/
	Automaton* S = Automaton::safetyClosure(this, f);
	out = S->isConstant(f);
	delete S;
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


void Automaton::top_safety_scc_recursive (Edge* edge, SetStd<Edge*>* done_edge, bool in_scc, int* done_symbol, weight_t* values, weight_t** value_symbol, int** counters) const {
	if (done_edge->contains(edge) == true) return;
	done_edge->insert(edge);

	unsigned int state_id = edge->getFrom()->getId();
	unsigned int symbol_id = edge->getSymbol()->getId();

	//printf("edge = %s\n", edge->toString().c_str());

	counters[state_id][symbol_id]--;
	if (counters[state_id][symbol_id] == 0) {
		/*printf("state %s done for %s\n",
				edge->getFrom()->getName().c_str(),
				edge->getSymbol()->getName().c_str()
		);*/

		for (Edge* succ : *(edge->getFrom()->getSuccessors(symbol_id))) {
			if (in_scc == false || succ->getFrom()->getTag() == succ->getTo()->getTag()) {
				weight_t tmp = std::min(succ->getWeight()->getValue(), values[succ->getTo()->getId()]);
				value_symbol[state_id][symbol_id] = std::max(value_symbol[state_id][symbol_id], tmp);
			}
		}
		done_symbol[state_id]--;

		/*printf("value_symbol[%s][%s] = %f\n",
				edge->getFrom()->getName().c_str(),
				edge->getSymbol()->getName().c_str(),
				value_symbol[state_id][symbol_id]
		);*/


		if (done_symbol[state_id] == 0) {
			//printf("state %s done\n", edge->getFrom()->toString().c_str());

			values[state_id] = value_symbol[state_id][symbol_id];
			for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
				values[state_id] = std::max(values[state_id], value_symbol[state_id][symbol_id]);
			}

			/*printf("values[%s] = %f\n",
					edge->getFrom()->getName().c_str(),
					values[state_id]
			);*/

			for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
				for (Edge* pred : *(edge->getFrom()->getPredecessors(symbol_id))) {
					if (in_scc == false || pred->getFrom()->getTag() == pred->getTo()->getTag()) {
						top_safety_scc_recursive(pred, done_edge, in_scc, done_symbol, values, value_symbol, counters);
					}
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
	int done_symbol[this->states->size()];
	int* counters[this->states->size()];
	weight_t* value_symbol[this->states->size()];
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		counters[state_id] = new int[this->alphabet->size()];
		value_symbol[state_id] = new weight_t[this->alphabet->size()];
		done_symbol[state_id] = this->alphabet->size();
		values[state_id] = this->max_domain;
		for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
			value_symbol[state_id][symbol_id] = this->min_domain;
			counters[state_id][symbol_id] = states->at(state_id)->getSuccessors(symbol_id)->size();
			for (Edge* edge : *(this->states->at(state_id)->getSuccessors(symbol_id))) {
				if (in_scc == false || edge->getFrom()->getTag() == edge->getTo()->getTag()){
					edges.at(edge->getWeight()->getId())->push(edge);
				}
			}
		}
	}

	//O((x+m)) because 'top_safety_scc_recursive' is called 2m times overall
	SetStd<Edge*> done_edge;
	for (unsigned int weight_id = 0; weight_id < this->weights->size(); ++weight_id) {
		while (edges.at(weight_id)->size() > 0) {
			Edge* edge = edges.at(weight_id)->head();
			edges.at(weight_id)->pop();
			top_safety_scc_recursive(edge, &done_edge, in_scc, done_symbol, values, value_symbol, counters);
		}
	}

	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		delete[] counters[state_id];
		delete[] value_symbol[state_id];
	}


	/*for (unsigned int id=0; id < this->states->size(); ++id) {
		printf("values[%u]=%s\n", id, std::to_string(values[id]).c_str());
	}*/
}





void Automaton::top_safety_tree (SCC_Tree* tree, weight_t* values, weight_t* top_values) const {
	top_values[tree->origin->getTag()] = values[tree->origin->getId()];
	for (auto iter = tree->nexts->begin(); iter != tree->nexts->end(); ++iter) {
		top_safety_tree(*iter, values, top_values);
	}
}



weight_t Automaton::top_Inf (weight_t* top_values) const {
	weight_t values[this->states->size()];
	top_safety_scc(values, false);
	top_safety_tree(this->SCCs_tree, values, top_values);
	return top_values[this->SCCs_tree->origin->getTag()];
}



weight_t Automaton::top_LimInf (weight_t* top_values) const {
	weight_t values[this->states->size()];
	top_safety_scc(values, true);
	top_safety_tree(this->SCCs_tree, values, top_values);
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
	weight_t infinity = std::max((weight_t)1, -(size*this->min_domain) + 1); // TODO

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
			return top_Inf(top_values);
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


weight_t Automaton::compute_Bottom (value_function_t f, weight_t* bot_values) {
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
				found = C->isIncludedIn(this, f);
				delete C;
			}

			return x;
		}
		else {
			fail("automata bot");
		}
	}
}


void Automaton::setMaxDomain (weight_t x) {
	this->max_domain = x;
}


void Automaton::setMinDomain (weight_t x) {
	this->min_domain = x;
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
	std::cout << "\tSCCs (" << this->nb_SCCs << "):";
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
	std::cout << "\n";
}





// -------------------------------- Nicolas -------------------------------- //





