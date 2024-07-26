#include <string>
#include <vector>
#include <memory>
#include <cassert>
#include <iomanip>
#include <limits>

#include "Automaton.h"
#include "Parser.h"
#include "Edge.h"
#include "utility.h"
#include "FORKLIFT/inclusion.h"


class SCC_Dag {
public:
	State* origin;
	SetStd<SCC_Dag*>* nexts;
	SCC_Dag() : origin(nullptr), nexts(new SetStd<SCC_Dag*>) {};
	void addNext (SCC_Dag* next) { this->nexts->insert(next); };
	~SCC_Dag() { delete nexts; }
	std::string toString (std::string offset) const {
		std::string s = "\n";
		s.append(offset);
		s.append(this->origin->getName());
		offset.append("\t");
		for (SCC_Dag* subdag : *nexts) s.append(subdag->toString(offset));
		return s;
	};
};



Automaton::~Automaton () {
	for (unsigned int state_id = 0; state_id < states->size(); ++state_id) {
		for (Symbol* symbol : *(states->at(state_id)->getAlphabet())) {
			for (Edge* edge : *(states->at(state_id)->getSuccessors(symbol->getId()))) {
				delete edge;
			}
		}
	}
	for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
		delete this->alphabet->at(symbol_id);
	}
	delete alphabet;
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		delete this->states->at(state_id);
	}
	delete states;
	for (unsigned int weight_id = 0; weight_id < this->weights->size(); ++weight_id) {
		delete this->weights->at(weight_id);
	}
	delete weights;
	for (unsigned int scc_id = 0; scc_id < this->nb_SCCs; ++scc_id) {
		delete this->SCCs[scc_id];
	}
	delete[] this->SCCs;
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
  // assert(isComplete() && "The automaton is not complete.");
  appropriateStates();
	compute_SCC();
}

void Automaton::appropriateStates() {
  for (auto *state : *states) {
    assert(state->automaton == nullptr);
    state->automaton = this;
  }
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


	for (const std::string &statename : parser->states) {
		State* state = new State(statename, this->alphabet->size(), this->min_domain, this->max_domain);
		this->states->insert(state->getId(), state);
		state_register.insert(state->getName(), state);
	}
	this->initial = state_register.at(parser->initial);

	for (const std::string &symbolname : parser->alphabet) {
		Symbol * symbol;
		if (sync_register.contains(symbolname))
			symbol = new Symbol(sync_register.at(symbolname));
		else
			symbol = new Symbol(symbolname);
		this->alphabet->insert(symbol->getId(), symbol);
		symbol_register.insert(symbol->getName(), symbol);
	}

	for (const auto &tuple : parser->edges) {
		Symbol* symbol = symbol_register.at(tuple.first.first);
		Weight* weight = weight_register.at(tuple.first.second);
		State* from = state_register.at(tuple.second.first);
		State* to = state_register.at(tuple.second.second);
		Edge *edge = new Edge(symbol, weight, from, to);
		from->addSuccessor(edge);
		to->addPredecessor(edge);
	}

  appropriateStates();

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
		case Inf: case LimInf : case LimInfAvg:
			sinkvalue = A->getMaxDomain();
			break;
		case Sup: case LimSup: case LimSupAvg:
			sinkvalue = A->getMinDomain();
			break;
		default: fail("case value function");
	}

	bool sinkFlag = false;
  /*
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
  */

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

  assert(that->isComplete() && "The automaton is not complete.");
	return that;
}



// -------------------------------- SCCs -------------------------------- //


void compute_SCC_dag (State* state, int* spot, int* low, bool* stackMem, SCC_Dag** SCCs) {
	if (stackMem[state->getId()] == true) return;
	stackMem[state->getId()] = true;

	if (spot[state->getId()] == low[state->getId()]) {
		SCCs[state->getTag()]->origin = state;
	}

	// printf("state: %s\n", state->getName().c_str());
	for (Symbol* symbol : *(state->getAlphabet())) {
		for (Edge* edge : *(state->getSuccessors(symbol->getId()))) {
			compute_SCC_dag(edge->getTo(), spot, low, stackMem, SCCs);
			if (state->getTag() != edge->getTo()->getTag()) {
				SCCs[state->getTag()]->addNext(SCCs[edge->getTo()->getTag()]);
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
		// printf("state %s, tag %d\n", state->getName().c_str(), *tag);
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

	this->SCCs = new SCC_Dag*[nb_SCCs];
	for (unsigned int scc_id = 0; scc_id < this->nb_SCCs; ++scc_id) {
		this->SCCs[scc_id] = new SCC_Dag();
	}
	compute_SCC_dag(this->initial, spot, low, stackMem, this->SCCs);

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
const std::string &Automaton::getName() const { return this->name; }
unsigned int Automaton::getAlphabetSize() const { return alphabet->size(); }



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
				auto value = ((edge->getWeight()->getValue() >= x) ? 1 : 0);
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
	weight_t newmin_domain = A->min_domain;
	weight_t newmax_domain = A->max_domain;

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
	if (A->isDeterministic() == false || f == Inf || f == LimInfAvg || f == LimSupAvg) {
		fail("invalid automaton type for liveness component (deterministic)");
	}

	State::RESET();
	Symbol::RESET();
	Weight::RESET();

	std::string newname = "LiveOf(" + A->getName() + ")";

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
	weight_t newmin_domain = A->min_domain;
	weight_t newmax_domain = A->max_domain;

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

	return new Automaton(newname, newalphabet, newstates, newweights, newmin_domain, newmax_domain, newinitial);
}



Automaton* Automaton::livenessComponent_prefixIndependent (const Automaton* A, value_function_t f) {
	weight_t top_values[A->nb_SCCs];
	SetList<Edge*>* scc_cycles[A->nb_SCCs];
	if (f == LimInf) {
		A->top_LimInf_cycles(top_values, scc_cycles);
	}
	else if (f == LimSup) {
		A->top_LimSup_cycles(top_values, scc_cycles);
	}
	else if (f == LimInfAvg || f == LimSupAvg) {
		A->top_LimAvg_cycles(top_values, scc_cycles);
	}
	else {
		fail("invalid automaton type for liveness component (nondeterministic)");
	}


	//Symbol::RESET();
	//Weight::RESET();
	State::RESET(A->states->size());

	std::string newname = "LiveOf(" + A->getName() + ")";

	MapArray<Symbol*>* newalphabet = new MapArray<Symbol*>(A->alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < A->alphabet->size(); ++symbol_id) {
		newalphabet->insert(symbol_id, new Symbol(A->alphabet->at(symbol_id)));
	}

	unsigned int size_of_cycles = 0;
	for (unsigned int scc_id = 0; scc_id < A->nb_SCCs; ++scc_id) {
		if (scc_cycles[scc_id]->size() != 0) {
			size_of_cycles += scc_cycles[scc_id]->size();
		}
	}

	MapArray<State*>* newstates = new MapArray<State*>(A->states->size() + size_of_cycles + 1);
	for (unsigned int state_id = 0; state_id < A->states->size(); ++state_id) {
		newstates->insert(state_id, new State(A->states->at(state_id)));
	}
	State* newinitial = newstates->at(A->initial->getId());

	MapStd<unsigned int, State*> state_register;
	for (unsigned int scc_id = 0; scc_id < A->nb_SCCs; ++scc_id) {
		if (scc_cycles[scc_id]->size() != 0) {
			for (Edge* edge : *(scc_cycles[scc_id])) {
				std::string statename = "copy_" + edge->getFrom()->getName();
				State* state = new State(statename, A->alphabet->size(), A->min_domain, A->max_domain);
				newstates->insert(state->getId(), state);
				state_register.insert(edge->getFrom()->getId(), state);
			}
		}
	}
	State* sink_state = new State("#sink#", A->alphabet->size(), A->min_domain, A->max_domain);
	newstates->insert(sink_state->getId(), sink_state);

	weight_t newmin_domain = A->min_domain;
	weight_t newmax_domain = A->max_domain;

	MapArray<Weight*>* newweights = new MapArray<Weight*>(A->weights->size());
	for (unsigned int weight_id = 0; weight_id < A->weights->size(); ++weight_id) {
		newweights->insert(weight_id, new Weight(A->weights->at(weight_id)));
	}
	Weight* lowest = newweights->at(0);
	Weight* greatest = newweights->at(newweights->size()-1);

	for (unsigned int state_id = 0; state_id < A->states->size(); ++state_id) {
		for (Symbol* symbol : *(A->states->at(state_id)->getAlphabet())) {
			for (Edge* edge : *(A->states->at(state_id)->getSuccessors(symbol->getId()))) {
				Symbol* new_symbol = newalphabet->at(symbol->getId());
				State* new_from = newstates->at(edge->getFrom()->getId());
				State* new_to = newstates->at(edge->getTo()->getId());
				Weight* new_weight = newweights->at(edge->getWeight()->getId());
				Edge* new_edge = new Edge(new_symbol, new_weight, new_from, new_to);
				new_from->addSuccessor(new_edge);
				new_to->addPredecessor(new_edge);
			}
		}
	}

	for (unsigned int scc_id = 0; scc_id < A->nb_SCCs; ++scc_id) {
		if (scc_cycles[scc_id]->size() != 0) {
			for (Edge* edge : *(scc_cycles[scc_id])) {
				Symbol* new_symbol;
				State* new_from;
				State* new_to;
				Weight* new_weight = greatest;
				Edge* new_edge;

				//edge of the cycle
				new_symbol = newalphabet->at(edge->getSymbol()->getId());
				new_from = state_register.at(edge->getFrom()->getId());
				new_to = state_register.at(edge->getTo()->getId());
				new_edge = new Edge(new_symbol, new_weight, new_from, new_to);
				new_from->addSuccessor(new_edge);
				new_to->addPredecessor(new_edge);

				//edge to the sink
				for (unsigned int symbol_id = 0; symbol_id < A->alphabet->size(); ++symbol_id) {
					new_symbol = newalphabet->at(symbol_id);
					new_from = state_register.at(edge->getFrom()->getId());
					new_to = sink_state;
					new_edge = new Edge(new_symbol, new_weight, new_from, new_to);
					new_from->addSuccessor(new_edge);
					new_to->addPredecessor(new_edge);
				}

				//edge from automaton
				new_symbol = newalphabet->at(edge->getSymbol()->getId());
				new_from = newstates->at(edge->getFrom()->getId());
				new_to = state_register.at(edge->getTo()->getId());
				new_edge = new Edge(new_symbol, new_weight, new_from, new_to);
				new_from->addSuccessor(new_edge);
				new_to->addPredecessor(new_edge);
			}
		}
	}

	for (unsigned int symbol_id = 0; symbol_id < A->alphabet->size(); ++symbol_id) {
		Symbol* new_symbol = newalphabet->at(symbol_id);
		State* new_from = sink_state;
		State* new_to = sink_state;
		Weight* new_weight = lowest;
		Edge* new_edge = new Edge(new_symbol, new_weight, new_from, new_to);
		new_from->addSuccessor(new_edge);
		new_to->addPredecessor(new_edge);
	}

	for (unsigned int scc_id = 0; scc_id < A->nb_SCCs; ++scc_id) {
		delete scc_cycles[scc_id];
	}

	Automaton* that = new Automaton(newname, newalphabet, newstates, newweights, newmin_domain, newmax_domain, newinitial);
	Automaton* AA = copy_trim_complete(that, f);
	delete that;
	return AA;
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
		return ((x->getValue() < y->getValue()) ? y : x);
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


/* old toLimSup
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
	case LimSup: case LimInfAvg: case LimSupAvg:
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
	MapArray<Weight*>* newweights = new MapArray<Weight*>(A->weights->size());
	for (unsigned int weight_id = 0; weight_id < A->weights->size(); ++weight_id) {
		newweights->insert(weight_id, new Weight(A->weights->at(weight_id)));
	}




	
	// collect the weights that actually occur in the new automaton
	std::set<unsigned int> tempWeightIds;
	for (const auto &edge : set_of_edges) {
		tempWeightIds.insert(edge.second.second.second->getId());
	}


//  std::map<unsigned, unsigned> newweights_mapping;
//	MapArray<Weight*>* newweights = new MapArray<Weight*>(tempWeightIds.size());
//	int ctr = 0;
//	for (auto weight_id : tempWeightIds) {
//		newweights->insert(ctr, new Weight(A->weights->at(weight_id)));
//    newweights_mapping[weight_id] = ctr;
//		ctr++;
//	}


	// MapArray<State*>* newstates = new MapArray<State*>(set_of_states.size());
	MapStd<pair<State*, Weight*>, State*> state_register;
	for (const pair<State*, Weight*> &weighted_state : set_of_states) {
		std::string statename = "(" + weighted_state.first->getName() + ", " + std::to_string(weighted_state.second->getValue()) + ")";
		State* state = new State(statename, newalphabet->size(), newmin_domain, newmax_domain);
		// newstates->insert(state->getId(), state);
		state_register.insert(weighted_state, state);
	}
	State* newinitial = state_register.at(start);

	for (const auto &edgeA : set_of_edges) {
		Symbol* symbol = newalphabet->at(edgeA.first->getId());
    // assert(newweights_mapping.count(edgeA.second.second.second->getId()) > 0);
		//Weight* weight = newweights->at(newweights_mapping[edgeA.second.second.second->getId()]);
		Weight* weight = newweights->at(edgeA.second.second.second->getId());
		State* from = state_register.at(edgeA.second.first);
		State* to = state_register.at(edgeA.second.second);
		Edge *edge = new Edge(symbol, weight, from, to);
		from->addSuccessor(edge);
		to->addPredecessor(edge);
	}




	// add a sink state if incomplete 
	bool isComplete = true;
	for (auto st : state_register) {
		for (auto sy : *newalphabet) {
			if (1 > st.second->getSuccessors(sy->getId())->size()) {
				isComplete = false;
				break;
			}
		}
		if (!isComplete) {
			break;
		}
	}

	MapArray<State*>* newstates;
	if (isComplete) {
		newstates = new MapArray<State*>(set_of_states.size());
		for (auto st : state_register) {
			newstates->insert(st.second->getId(), st.second);
		}
	}
	else {
		newstates = new MapArray<State*>(set_of_states.size() + 1);
		for (auto st : state_register) {
			newstates->insert(st.second->getId(), st.second);
		}
		State* sinkstate = new State("sink", newalphabet->size(), newmin_domain, newmax_domain);
		newstates->insert(sinkstate->getId(), sinkstate);
		Weight* sinkweight = newweights->at(initWeightId);
		
		for (auto st : *newstates) {
			for (auto sy : *newalphabet) {
				if (1 > st->getSuccessors(sy->getId())->size()) {
					Edge* sinkedge = new Edge(sy, sinkweight, st, sinkstate);
					st->addSuccessor(sinkedge);
					sinkstate->addPredecessor(sinkedge);
				}
			}
		}
	}

	return new Automaton(newname, newalphabet, newstates, newweights, newmin_domain, newmax_domain, newinitial);
}
*/

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
	case LimSup: case LimInfAvg: case LimSupAvg:
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

	weight_t newmin_domain = A->min_domain;
	weight_t newmax_domain = A->max_domain;

	SetStd<pair<State*, Weight*>> set_of_states;
	SetStd<pair<Symbol*, pair<pair<State*, Weight*>, pair<State*, Weight*>>>> set_of_edges;
	auto start = pair<State*, Weight*>(A->initial, A->weights->at(initWeightId));
	explore(start, set_of_states, set_of_edges);

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

	Automaton* that = new Automaton(newname, newalphabet, newstates, newweights, newmin_domain, newmax_domain, newinitial);
	Automaton* AA = copy_trim_complete(that, f);
	delete that;
	return AA;
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


Automaton* Automaton::determinizeInf (const Automaton* A) {
	State::RESET();
	Symbol::RESET();
	Weight::RESET();
	
	MapArray<Symbol*>* newalphabet = new MapArray<Symbol*>(A->alphabet->size());
	for (unsigned int symbol_id = 0; symbol_id < A->alphabet->size(); ++symbol_id) {
		newalphabet->insert(symbol_id, new Symbol(A->alphabet->at(symbol_id)));
	}

	weight_t top_value_of_A = A->getTopValue(Inf);
	Weight* top_weight_of_A = nullptr;
	MapArray<Weight*>* newweights = new MapArray<Weight*>(A->weights->size());
	for (unsigned int weight_id = 0; weight_id < A->weights->size(); ++weight_id) {
		Weight* weight = new Weight(A->weights->at(weight_id));
		newweights->insert(weight_id, weight);
		if (weight->getValue() == top_value_of_A) {
			top_weight_of_A = weight;
		}
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

	std::vector<unsigned int> funcFrom(n, 0);
	funcFrom[A->getInitial()->getId()] = top_weight_of_A->getId();
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

bool Automaton::isNonEmpty (value_function_t f, weight_t x ) {
	return (getTopValue(f) >= x);
}

bool Automaton::isUniversal (value_function_t f, weight_t x)  {
	return (getBottomValue(f) >= x);
}

bool Automaton::isComplete () const {
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
			if (1 > this->states->at(state_id)->getSuccessors(symbol_id)->size()) return false;
		}
	}
	return true;
}


bool Automaton::isLimAvgConstant() const {
	weight_t top = getTopValue(LimSupAvg);//top of LimSupAvg and LimInfAvg coincide

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
            weight_t du = dist[u];
            if (du + value < dist[v]) {
              dist[v] = static_cast<int>(du + value);
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
				auto value = (((edge->getWeight()->getValue() - dist[u] + dist[v]) == top) ? 1 : 0);
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
	delete Dist;
	return out;
}


bool Automaton::isConstant (value_function_t f) {
	if ((f == LimSupAvg || f == LimInfAvg) && isDeterministic() == false) {
		return isLimAvgConstant();
	}
	else {
		return (getTopValue(f) == getBottomValue(f));
	}
}

bool Automaton::isIncludedIn(const Automaton* B, value_function_t f, bool booleanized) {
    assert(alphabetsAreCompatible(B) && "Incompatible alphabets");

    if (booleanized == true) {
        return isIncludedIn_booleanized(B, f);
    }

    return isIncludedIn_antichains(B, f);
}

bool Automaton::isIncludedIn_antichains(const Automaton* B, value_function_t f) {
	if (f == LimSupAvg || f == LimInfAvg) {
		if (B->isDeterministic()) {
			Automaton* C = Automaton::product(this, Minus, B);
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
    // unique_ptr to keep memory in case we create new limSup automata
    std::unique_ptr<Automaton> limSupThisMem, limSupBMem;
    const Automaton *limSupThis{nullptr}, *limSupB{nullptr};

    if (f == LimSup) {
      limSupThis = this;
      limSupB = B;
    } else {
      limSupThisMem = std::unique_ptr<Automaton>(Automaton::toLimSup(this, f));
      limSupBMem = std::unique_ptr<Automaton>(Automaton::toLimSup(B, f));
      limSupThis = limSupThisMem.get();
      limSupB = limSupBMem.get();
    }

    for (auto *weight: *weights) {
        auto boolA = std::unique_ptr<Automaton>(booleanize(limSupThis, weight->getValue()));
        auto boolB = std::unique_ptr<Automaton>(booleanize(limSupB, weight->getValue()));

        if (!inclusion(boolA.get(), boolB.get())) {
          return false;
        }
    }

    return true;
}


bool Automaton::isSafe (value_function_t f) {
	if (f == Inf) {
		return true;
	}

	Automaton* S = Automaton::safetyClosure(this, f);
	bool out;

	if ((f == LimSupAvg || f== LimInfAvg) && !this->isDeterministic()) {
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

	if (f == Inf) {
		return this->isConstant(Inf);
	}

	Automaton* S = Automaton::safetyClosure(this, f);
	out = S->isConstant(f);
	delete S;
	return out;
}










// -------------------------------- Tops -------------------------------- //

void Automaton::top_dag (SCC_Dag* dag, bool* done, weight_t* top_values) const {
	if (done[dag->origin->getTag()] == true) return;
	done[dag->origin->getTag()] = true;

	for (SCC_Dag* subdag : *(dag->nexts)) {
		top_dag(subdag, done, top_values);
		top_values[dag->origin->getTag()] = std::max(top_values[dag->origin->getTag()],
				top_values[subdag->origin->getTag()]);
	}
}


void Automaton::top_reachably_scc (State* state, bool in_scc, bool* spot, weight_t* values) const {
	if (spot[state->getId()] == true) return;
	spot[state->getId()] = true;
	for (Symbol* symbol : *(state->getAlphabet())) {
		for (Edge* edge : *(state->getSuccessors(symbol->getId()))) {
			if (edge->getTo()->getTag() == state->getTag()) {
				top_reachably_scc(edge->getTo(), in_scc, spot, values);
				values[state->getId()] = std::max(values[state->getId()], edge->getWeight()->getValue());
				values[state->getId()] = std::max(values[state->getId()], values[edge->getTo()->getId()]);
			}
			else if (in_scc == false) {
				values[state->getId()] = std::max(values[state->getId()], edge->getWeight()->getValue());
			}
		}
	}
}


weight_t Automaton::top_reachably (bool in_scc, weight_t* values, weight_t* top_values) const {
	bool spot[this->states->size()];
	bool done[this->nb_SCCs];

	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		values[state_id] = this->min_domain;
		spot[state_id] = false;
	}

	for (unsigned int scc_id = 0; scc_id < this->nb_SCCs; ++scc_id) {
		top_reachably_scc(this->SCCs[scc_id]->origin, in_scc, spot, values);
		done[scc_id] = false;
		top_values[scc_id] = values[this->SCCs[scc_id]->origin->getId()];
	}

	top_dag(this->SCCs[this->initial->getTag()], done, top_values);
	return top_values[this->initial->getTag()];
}


weight_t Automaton::top_Sup (weight_t* top_values) const {
	weight_t values[this->states->size()];
	return top_reachably(false, values, top_values);
}


weight_t Automaton::top_LimSup (weight_t* top_values) const {
	weight_t values[this->states->size()];
	return top_reachably(true, values, top_values);
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
		done_symbol[state_id] = 0;
		for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
			value_symbol[state_id][symbol_id] = this->min_domain;
			counters[state_id][symbol_id] = 0;

			bool flag = false;
			for (Edge* edge : *(this->states->at(state_id)->getSuccessors(symbol_id))) {
				if (in_scc == false || edge->getFrom()->getTag() == edge->getTo()->getTag()){
					edges.at(edge->getWeight()->getId())->push(edge);
					counters[state_id][symbol_id]++;
					flag = true;
				}
			}
			if (flag == true) done_symbol[state_id]++;
		}
		if (done_symbol[state_id] == 0) {
			values[state_id] = this->min_domain;
		}
		else {
			values[state_id] = this->max_domain;
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


/*
weight_t Automaton::top_safety (bool in_scc, weight_t* values, weight_t* top_values) const {
	bool done[this->nb_SCCs];
	top_safety_scc(values, in_scc);

	for (unsigned int scc_id = 0; scc_id < this->nb_SCCs; ++scc_id) {
		done[scc_id] = false;
		top_values[scc_id] = values[this->SCCs[scc_id]->origin->getId()];
	}
	top_dag(this->SCCs[this->initial->getTag()], done, top_values);
	return top_values[this->initial->getTag()];
}
*/



weight_t Automaton::top_Inf (weight_t* top_values) const {
	weight_t values[this->states->size()];
	bool done[this->nb_SCCs];
	top_safety_scc(values, true);

	for (unsigned int scc_id = 0; scc_id < this->nb_SCCs; ++scc_id) {
		done[scc_id] = false;
		top_values[scc_id] = values[this->SCCs[scc_id]->origin->getId()];
	}

	top_dag(this->SCCs[this->initial->getTag()], done, top_values);
	return top_values[this->initial->getTag()];
}


weight_t Automaton::top_LimInf (weight_t* top_values) const {
	weight_t values[this->states->size()];
	bool done[this->nb_SCCs];
	top_safety_scc(values, false);

	for (unsigned int scc_id = 0; scc_id < this->nb_SCCs; ++scc_id) {
		done[scc_id] = false;
		top_values[scc_id] = this->min_domain;
	}

	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		top_values[this->states->at(state_id)->getTag()] = std::max(
				top_values[this->states->at(state_id)->getTag()],
				values[state_id]
		);
	}

	top_dag(this->SCCs[this->initial->getTag()], done, top_values);
	return top_values[this->initial->getTag()];
}




weight_t Automaton::top_LimAvg (weight_t* top_values) const {
	unsigned int size = this->states->size();
	weight_t distance[size + 1][size];
	weight_t infinity = std::max(weight_t(1), -(weight_t(size)*this->min_domain) + 1); // TODO

	// O(n)
	for (unsigned int length = 0; length <= size; ++length) {
		for (unsigned int state_id = 0; state_id < size; ++state_id) {
			distance[length][state_id] = infinity;
		}
	}


	//O(n)
	auto initialize_distances = [] (SCC_Dag* dag, weight_t* distance, auto &rec) -> void {
		distance[dag->origin->getId()] = 0;
		for (auto iter = dag->nexts->begin(); iter != dag->nexts->end(); ++iter) {
			rec(*iter, distance, rec);
		}
	};
	initialize_distances(this->SCCs[this->initial->getTag()], distance[0], initialize_distances);


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
	bool done[this->nb_SCCs];
	for (unsigned int scc_id = 0; scc_id < this->nb_SCCs; ++scc_id) {
		done[scc_id] = false;
		top_values[scc_id] = this->min_domain;
	}

	for (unsigned int state_id = 0; state_id < size; ++state_id) {
		weight_t min_lenght_avg = this->max_domain;
		bool len_flag = false;
		if (distance[size][state_id] != infinity) { // => id has an ongoing edge (inside its SCC)
			for (unsigned int lenght = 0; lenght < size; ++lenght) { // hence the nested loop is call at most O(m) times
				if (distance[lenght][state_id] != infinity) {
					weight_t avg = (distance[lenght][state_id] - distance[size][state_id] + 0.0) / weight_t(size - lenght + 0.0);
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

	top_dag(this->SCCs[this->initial->getTag()], done, top_values);
	return top_values[this->initial->getTag()];
}



void Automaton::top_LimAvg_cycles (weight_t* top_values, SetList<Edge*>** scc_cycles) const {
	unsigned int size = this->states->size();
	Edge* back_distance[size + 1][size];
	weight_t distance[size + 1][size];
	weight_t infinity = std::max(weight_t(1), -(this->min_domain*weight_t(size)) + 1);

	// O(n)
	for (unsigned int length = 0; length <= size; ++length) {
		for (unsigned int state_id = 0; state_id < size; ++state_id) {
			distance[length][state_id] = infinity;
			back_distance[length][state_id] = nullptr;
		}
	}


	//O(n)
	auto initialize_distances = [] (SCC_Dag* dag, weight_t* distance, auto &rec) -> void {
		distance[dag->origin->getId()] = 0;
		for (auto iter = dag->nexts->begin(); iter != dag->nexts->end(); ++iter) {
			rec(*iter, distance, rec);
		}
	};
	initialize_distances(this->SCCs[this->initial->getTag()], distance[0], initialize_distances);


	// O(n.m)
	for (unsigned int len = 1; len <= size; ++len) {
		for (unsigned int state_id = 0; state_id < size; ++state_id)	{
			for (Symbol* symbol : *(states->at(state_id)->getAlphabet())) {
				for (Edge* edge : *(states->at(state_id)->getSuccessors(symbol->getId()))) {
					if (edge->getFrom()->getTag() == edge->getTo()->getTag()) {
						if (distance[len-1][edge->getFrom()->getId()] != infinity) {
							weight_t old_value = distance[len-1][edge->getTo()->getId()];
							weight_t new_value = distance[len-1][edge->getFrom()->getId()] - edge->getWeight()->getValue();
							if (old_value == infinity || new_value < old_value) {
								distance[len][edge->getTo()->getId()] = new_value;
								back_distance[len][edge->getTo()->getId()] = edge;
							}
						}
					}
				}
			}
		}
	}

	//O(n.m)
	bool done[this->nb_SCCs];
	State* scc_back[this->nb_SCCs];
	weight_t scc_values[this->nb_SCCs];
	for (unsigned int scc_id = 0; scc_id < this->nb_SCCs; ++scc_id) {
		done[scc_id] = false;
		top_values[scc_id] = this->min_domain;
		scc_values[scc_id] = this->min_domain;
		scc_back[scc_id] = nullptr;
		scc_cycles[scc_id] = new SetList<Edge*>();
	}

	for (unsigned int state_id = 0; state_id < size; ++state_id) {
		weight_t min_lenght_avg = this->max_domain;
		bool len_flag = false;
		if (distance[size][state_id] != infinity) { // => id has an ongoing edge (inside its SCC)
			for (unsigned int length = 0; length < size; ++length) { // hence the nested loop is call at most O(m) times
				if (distance[length][state_id] != infinity) {
					weight_t avg = (distance[length][state_id] - distance[size][state_id] + 0.0) / (size - length + 0.0);
					min_lenght_avg = std::min(min_lenght_avg, avg);
					len_flag = true;
				}
			}
		}
		if (len_flag && top_values[this->states->at(state_id)->getTag()] < min_lenght_avg) {
			top_values[this->states->at(state_id)->getTag()] = min_lenght_avg;
			scc_values[this->states->at(state_id)->getTag()] = min_lenght_avg;
			scc_back[this->states->at(state_id)->getTag()] = this->states->at(state_id);
		}
	}

	top_dag(this->SCCs[this->initial->getTag()], done, top_values);

	for (unsigned int scc_id = 0; scc_id < this->nb_SCCs; ++scc_id) {
		if (scc_values[scc_id] == top_values[scc_id]){
			State* seek_state = scc_back[scc_id];
			int length = this->states->size();
			while(seek_state != nullptr && distance[0][seek_state->getId()] == 1) {
				distance[0][seek_state->getId()] = 1;//spot[seek_state] = true
				seek_state = back_distance[length][seek_state->getId()]->getFrom();
				length--;
			}
			//if (seek_state == nullptr) continue;//impossible?

			State* state = scc_back[scc_id];
			length = this->states->size();
			while (state != seek_state) {
				state = back_distance[length][state->getId()]->getFrom();
				length--;
			}
			do {
				scc_cycles[scc_id]->push(back_distance[length][state->getId()]);
				state = back_distance[length][state->getId()]->getFrom();
				length--;
			} while (state != seek_state);
		}
	}
}


State* Automaton::top_cycles_explore (State* state, bool* spot, weight_t (*filter)(weight_t,weight_t), weight_t* top_values, SetList<Edge*>** scc_cycles) const {
	if (spot[state->getId()] == true) return state;

	spot[state->getId()] = true;
	for (Symbol* symbol : *(state->getAlphabet())){
		for (Edge* edge : *(state->getSuccessors(symbol->getId()))){
			if (scc_cycles[state->getTag()]->size() == 0) {
				if (edge->getFrom()->getTag() == edge->getTo()->getTag()) {
					weight_t value = filter(edge->getWeight()->getValue(), top_values[edge->getTo()->getTag()]);
					if (value == top_values[edge->getTo()->getTag()]) {
						State* seek_state = top_cycles_explore(edge->getTo(), spot, filter, top_values, scc_cycles);
						if (seek_state != nullptr) {
							scc_cycles[seek_state->getTag()]->push(edge);
							if (seek_state != state) {
								return seek_state;
							}
							else {
								return nullptr;
							}
						}
					}
				}
			}
		}
	}

	return nullptr;
}


void Automaton::top_cycles (weight_t (*filter)(weight_t,weight_t), weight_t* scc_values, weight_t* top_values, SetList<Edge*>** scc_cycles) const {
	bool spot[this->states->size()];

	for (unsigned int state_id = 0 ; state_id < this->states->size(); ++state_id) {
		spot[state_id] = false;
	}

	for (unsigned int scc_id = 0 ; scc_id < this->nb_SCCs; ++scc_id) {
		scc_cycles[scc_id] = new SetList<Edge*>();
	}

	for (unsigned int state_id = 0 ; state_id < this->states->size(); ++state_id) {
		if (this->states->at(state_id)->getTag() == -1) continue;
		if (scc_values[this->states->at(state_id)->getId()] != top_values[this->states->at(state_id)->getTag()]) continue;
		if (scc_cycles[this->states->at(state_id)->getTag()]->size() != 0) continue;
		top_cycles_explore(this->states->at(state_id), spot, filter, top_values, scc_cycles);
	}
}


/*
void Automaton::top_LimInf_cycles (weight_t* top_values, SetList<Edge*>** scc_cycles) const {
	weight_t (*filter)(weight_t,weight_t) = [] (weight_t x, weight_t y) -> weight_t {
		return std::max(x, y);
	};
	weight_t scc_values[this->states->size()];
	top_safety(true, scc_values, top_values);
	top_cycles(filter, scc_values, top_values, scc_cycles);
}
*/

void Automaton::top_LimInf_cycles (weight_t* top_values, SetList<Edge*>** scc_cycles) const {
	weight_t (*filter)(weight_t,weight_t) = [] (weight_t x, weight_t y) -> weight_t {
		return std::max(x, y);
	};
	weight_t scc_values[this->states->size()];

	bool done[this->nb_SCCs];
	top_safety_scc(scc_values, true);

	for (unsigned int scc_id = 0; scc_id < this->nb_SCCs; ++scc_id) {
		done[scc_id] = false;
		top_values[scc_id] = this->min_domain;
	}

	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		top_values[this->states->at(state_id)->getTag()] = std::max(
				top_values[this->states->at(state_id)->getTag()],
				scc_values[state_id]
		);
	}

	top_dag(this->SCCs[this->initial->getTag()], done, top_values);
	top_cycles(filter, scc_values, top_values, scc_cycles);
}


void Automaton::top_LimSup_cycles (weight_t* top_values, SetList<Edge*>** scc_cycles) const {
	weight_t (*filter)(weight_t,weight_t) = [] (weight_t x, weight_t y) -> weight_t {
		return std::min(x, y);
	};
	weight_t scc_values[this->states->size()];
	top_reachably(true, scc_values, top_values);
	top_cycles(filter, scc_values, top_values, scc_cycles);
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
		case LimInfAvg: case LimSupAvg:
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



bool Automaton::alphabetsAreCompatible(const Automaton *B) const {
  if (alphabet->size() != B->alphabet->size()) {
    return false;
  }

  // same symbols must be at same indices
  for (auto *symbol1 : *alphabet) {
    auto *symbol2 = B->alphabet->at(symbol1->getId());
    if (symbol1->getName() != symbol2->getName()) {
      return false;
    }
  }

  return true;
}




// -------------------------------- toStrings -------------------------------- //


void Automaton::print (bool full) const {
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
	std::cout << this->SCCs[this->initial->getTag()]->toString("\t\t") << "\n";
	unsigned int nb_edge = 0;
	for (unsigned int state_id = 0; state_id < states->size(); ++state_id) {
		for (Symbol* symbol : *(states->at(state_id)->getAlphabet())) {
			nb_edge += states->at(state_id)->getSuccessors(symbol->getId())->size();
		}
	}
	std::cout << "\tedges (" << nb_edge << "):\n";
	for (unsigned int state_id = 0; state_id < states->size(); ++state_id) {
		for (Symbol* symbol : *(states->at(state_id)->getAlphabet())) {
            auto *state = states->at(state_id);
            for (auto *edge : *state->getSuccessors(symbol->getId())) {
                std::cout << "\t\t" << edge->getSymbol()->toString() << " : ";
                if (full) {
                  std::cout << std::setprecision(std::numeric_limits<weight_t::T>::max_digits10)
                            << std::fixed;
                }
                std::cout << *edge->getWeight()->getValue() << ", "
                          << edge->getFrom()->getName() << " -> "
                          << edge->getTo()->getName() << "\n";
            }
		}
	}
	std::cout << "\n";
}





// -------------------------------- Nicolas -------------------------------- //





