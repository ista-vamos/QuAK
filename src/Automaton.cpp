#include <string>
#include <vector>
#include <memory>
#include <cassert>
#include <iomanip>
#include <limits>
#include <algorithm>

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

	// if there are unreachable states, construct a new automaton ignoring those states
	unsigned int reachable = 0;
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		if (this->states->at(state_id)->getTag() > -1) {
			reachable++;
		}
	}
	if (reachable < this->states->size()) {
		Parser parserTrim = this->parse_trim();
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
		delete this->alphabet;
		for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
			delete this->states->at(state_id);
		}
		delete this->states;
		for (unsigned int weight_id = 0; weight_id < this->weights->size(); ++weight_id) {
			delete this->weights->at(weight_id);
		}
		delete this->weights;
		for (unsigned int scc_id = 0; scc_id < this->nb_SCCs; ++scc_id) {
			delete this->SCCs[scc_id];
		}
		delete[] this->SCCs;
		build(newname, &parserTrim, sync_register);
	}
}

Parser Automaton::parse_trim() {
    Parser parser(this->getMinDomain(), this->getMaxDomain());
    for (unsigned int stateA_id = 0; stateA_id < this->getStates()->size(); ++stateA_id) {
    	if (this->getStates()->at(stateA_id)->getTag() == -1) {
			continue;
		}
		parser.states.insert(this->getStates()->at(stateA_id)->getName());
        for (Symbol* symbol : *(this->getStates()->at(stateA_id)->getAlphabet())) {
			parser.alphabet.insert(symbol->getName());
			for (Edge* edgeA : *(this->getStates()->at(stateA_id)->getSuccessors(symbol->getId()))) {
				parser.weights.insert(edgeA->getWeight()->getValue());
				std::pair<std::pair<std::string, weight_t>,std::pair<std::string, std::string>> edge;
				edge.first.first = edgeA->getSymbol()->getName();
				edge.first.second = edgeA->getWeight()->getValue();
				edge.second.first = edgeA->getFrom()->getName();
				edge.second.second = edgeA->getTo()->getName();
				parser.edges.insert(edge);
			}
	    }
    }
    parser.initial = this->getInitial()->getName();
    return parser;
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
		default: QUAK_FAIL("case aggregator_t");
	}
}

weight_t aggregator_apply (aggregator_t aggregator, weight_t x, weight_t y) {
	switch (aggregator) {
		case Max: return std::max(x, y);
		case Min: return std::min(x, y);
		case Plus: return x + y;
		case Minus: return x - y;
		case Times: return x * y;
		default: QUAK_FAIL("case aggregator_t");
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
				if (B->alphabet->at(symbol->getId())->getName() != symbol->getName()) QUAK_FAIL("product with unsynchronized alphabet");

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
		default: QUAK_FAIL("case value function");
	}

	bool sinkFlag = false;

	for (unsigned int stateA_id = 0; stateA_id < A->getStates()->size(); ++stateA_id) {
		if (A->getStates()->at(stateA_id)->getTag() == -1) continue;
		for (unsigned int symbol_id = 0; symbol_id < A->getAlphabet()->size(); ++symbol_id) {
			if (parser->alphabet.contains(A->getAlphabet()->at(symbol_id)->getName()) == false) continue;
			if (A->getStates()->at(stateA_id)->getSuccessors(symbol_id)->size() > 0) continue;
			// if (A->getStates()->at(stateA_id)->getAlphabet()->contains(A->getAlphabet()->at(symbol_id)) == true) continue;
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

weight_t Automaton::getTopValue (value_function_t f, UltimatelyPeriodicWord** witness) const {
	weight_t *top_values = new weight_t[this->nb_SCCs];
	weight_t top = compute_Top(f, top_values, witness);
	/*for (unsigned int id = 0;id <this->nb_SCCs; id++) {
		printf("top[%u] = %s\n", id, std::to_string(top_values[id]).c_str());
	}*/
    delete top_values;
	return top;
}

weight_t Automaton::getBottomValue (value_function_t f, UltimatelyPeriodicWord** witness) {
	weight_t *bot_values = new weight_t[this->nb_SCCs];
	auto bot = compute_Bottom(f, bot_values, witness);
    delete bot_values;
    return bot;
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



Automaton* Automaton::safetyClosure(Automaton* A, value_function_t f) {
	if (f == Sup) {
		std::unique_ptr<Automaton> AA = std::unique_ptr<Automaton>(Automaton::toLimSup(A, f));
		return safetyClosure(AA.get(), LimSup);
	}

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

	weight_t *top_values = new weight_t[A->nb_SCCs];
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

    delete top_values;
	return new Automaton(newname, newalphabet, newstates, newweights, newmin_domain, newmax_domain, newinitial);
}



Automaton* Automaton::livenessComponent_deterministic (const Automaton* A, value_function_t f) {
	if (A->isDeterministic() == false || f == Inf || f == LimInfAvg || f == LimSupAvg) {
		QUAK_FAIL("invalid automaton type for deterministic liveness component");
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

	weight_t *top_values = new weight_t[A->nb_SCCs];
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

    delete top_values;
	return new Automaton(newname, newalphabet, newstates, newweights, newmin_domain, newmax_domain, newinitial);
}


Automaton* Automaton::livenessComponent(const Automaton* A, value_function_t f) {
	if ((f == Sup || f == LimInf || f == LimSup) && A->isDeterministic()) {
		return Automaton::livenessComponent_deterministic(A, f);
	}
	
	if (f == LimInf || f == LimSup || f == LimInfAvg || f == LimSupAvg) {
		return Automaton::livenessComponent_prefixIndependent(A, f);
	}
	
	QUAK_FAIL("Cannot do safety-liveness decomposition for this type of automata.");
}



Automaton* Automaton::livenessComponent_prefixIndependent (const Automaton* A, value_function_t f) {
	weight_t *top_values = new weight_t[A->nb_SCCs];
	SetList<Edge*>* scc_cycles[A->nb_SCCs];
	if (f == LimInf) {
		A->top_LimInf_cycles(top_values, scc_cycles);
        delete[] top_values;
	}
	else if (f == LimSup) {
		A->top_LimSup_cycles(top_values, scc_cycles);
        delete[] top_values;
	}
	else if (f == LimInfAvg || f == LimSupAvg) {
		A->top_LimAvg_cycles(top_values, scc_cycles);
        delete[] top_values;
	}
	else {
        delete[] top_values;
		QUAK_FAIL("invalid automaton type for prefix-independent liveness component computation");
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
					// new_edge = new Edge(new_symbol, new_weight, new_from, new_to);
					new_edge = new Edge(new_symbol, lowest, new_from, new_to);
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

	Automaton* that = new Automaton(newname, newalphabet, newstates, newweights, newmin_domain, newmax_domain, newinitial);
	Automaton* AA = copy_trim_complete(that, f);

	// AA->print();

	for (unsigned int scc_id = 0; scc_id < A->nb_SCCs; ++scc_id) {
		delete scc_cycles[scc_id];
	}

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
		QUAK_FAIL("invalid translation to LimSup");
	default:
		QUAK_FAIL("invalid value function"); }

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
	Automaton* AA = copy_trim_complete(that, LimSup);
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

bool Automaton::isNonEmpty (value_function_t f, weight_t x, UltimatelyPeriodicWord** witness) {
	return (getTopValue(f, witness) >= x);
}


bool Automaton::isUniversal (value_function_t f, weight_t x, UltimatelyPeriodicWord** witness)  {
	Automaton* C = Automaton::constantAutomaton(this, x);
	bool flag = C->isIncludedIn(this, f, false, witness);
	delete C;
	return flag;
}

bool Automaton::isComplete () const {
	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		for (unsigned int symbol_id = 0; symbol_id < this->alphabet->size(); ++symbol_id) {
			if (1 > this->states->at(state_id)->getSuccessors(symbol_id)->size()) return false;
		}
	}
	return true;
}


bool Automaton::isLimAvgConstant(UltimatelyPeriodicWord** witness) const {
	weight_t top = getTopValue(LimSupAvg); //top of LimSupAvg and LimInfAvg coincide

    weight_t dist[this->getStates()->size()];
    for (unsigned int state_id = 0; state_id < this->getStates()->size(); ++state_id) {
        dist[state_id] = 0;
    }

	for (unsigned int len = 0; len < this->getStates()->size(); len++) {
		for (unsigned int state_id = 0; state_id < this->getStates()->size(); ++state_id) {
			for (Symbol* symbol : *(this->getStates()->at(state_id)->getAlphabet())) {
				for (Edge* edge : *(this->getStates()->at(state_id)->getSuccessors(symbol->getId()))) {
					unsigned int u = edge->getFrom()->getId();
					unsigned int v = edge->getTo()->getId();
					//weights are inversed AND shifted by 'top': -(edge-top) = top-edge
					weight_t value = top - edge->getWeight()->getValue();
					weight_t du = dist[u];
					if (du + value < dist[v]) {
						dist[v] = (du + value);
					}
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
				weight_t x = (edge->getWeight()->getValue() - dist[u] + dist[v]);
			}
		}
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
				//originally: -(edge-top) + from - to = 0 (<=?)
				//inverted weights: (edge-top) - from + to = 0
				//equivalently: edge - from + to = top
				weight_t x = (edge->getWeight()->getValue() - dist[u] + dist[v]);
				auto value = ((x == top) ? 1 : 0);
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
	// Dist->print();
	bool out = Dist->isUniversal(LimInf, 1, witness);
	delete Dist;
	return out;
}

// witness for isConstant is a lasso word whose value is strictly less than the automaton's top value
bool Automaton::isConstant (value_function_t f, UltimatelyPeriodicWord** witness) {
	if (isDeterministic() == true) {
		return (getTopValue(f) == getBottomValue(f, witness));
	}
	else if ((f == LimSupAvg || f == LimInfAvg)) {
		return isLimAvgConstant(witness);
	}
	else {
		return isUniversal(f, getTopValue(f), witness);
	}
}

bool Automaton::isEquivalentTo (const Automaton* B, value_function_t f, bool booleanized, UltimatelyPeriodicWord** witness) const {
	return this->isIncludedIn(B, f, booleanized, witness) && B->isIncludedIn(this, f, booleanized, witness);
}

bool Automaton::isIncludedIn(const Automaton* B, value_function_t f, bool booleanized, UltimatelyPeriodicWord** witness) const {
    assert(alphabetsAreCompatible(B) && "Incompatible alphabets");

    if (booleanized == true) {
        return isIncludedIn_booleanized(B, f, witness);
    }

    return isIncludedIn_antichains(B, f, witness);
}

bool Automaton::isIncludedIn_antichains(const Automaton* B, value_function_t f, UltimatelyPeriodicWord** witness) const {
	if (f == LimSupAvg || f == LimInfAvg) {
		if (B->isDeterministic()) {
			Automaton* C = Automaton::product(this, Minus, B);
			weight_t Ctop = C->getTopValue(f, witness);
			delete C;
			return (Ctop <= 0);
		}
		else {
			QUAK_FAIL("automata inclusion undecidable for nondeterministic limavg");
		}
	}
	else if (f == Inf || f == Sup || f == LimInf || f == LimSup) {
		bool flag;

		if (f == LimSup) {
			flag = inclusion(this, B, witness);
		}
		else {
			Automaton* AA = Automaton::toLimSup(this, f);
			Automaton* BB = Automaton::toLimSup(B, f);
			// AA->print();
			// BB->print();

			flag = inclusion(AA, BB, witness);
			delete AA;
			delete BB;
		}

		return flag;
	}
	else {
		QUAK_FAIL("automata inclusion type");
	}
}

bool Automaton::isIncludedIn_booleanized(const Automaton* B, value_function_t f, UltimatelyPeriodicWord** witness) const {
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

        if (!inclusion(boolA.get(), boolB.get(), witness)) {
          return false;
        }
    }

    return true;
}


bool Automaton::isSafe (value_function_t f, UltimatelyPeriodicWord** witness) {
	if (f == Inf) {
		return true;
	}

	Automaton* S = Automaton::safetyClosure(this, f);
	bool out;

	if ((f == LimSupAvg || f== LimInfAvg) && !this->isDeterministic()) {
		Automaton* SS = Automaton::determinizeInf(S);
		Automaton* C = Automaton::product(this, Minus, SS);
		out = C->isLimAvgConstant(witness);
		delete SS;
		delete C;
	}
	else {
		out = S->isIncludedIn(this, f, false, witness);
	}

	delete S;
	return out;
}

bool Automaton::isLive (value_function_t f, UltimatelyPeriodicWord** witness) {
	bool out;

	if (f == Inf) {
		return this->isConstant(Inf, witness);
	}

	Automaton* S = Automaton::safetyClosure(this, f);
	out = S->isConstant(f, witness);
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

	for (unsigned int weight_id = 0; weight_id < this->weights->size(); ++weight_id) {
		delete edges.at(weight_id);
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
	// top_safety_scc(values, true);
	top_safety_scc(values, false);

	for (unsigned int scc_id = 0; scc_id < this->nb_SCCs; ++scc_id) {
		top_values[scc_id] = values[this->SCCs[scc_id]->origin->getId()];
	}
	
	return values[this->initial->getId()];
}


// weight_t Automaton::top_Inf (weight_t* top_values) const {
// 	weight_t values[this->states->size()];
// 	bool done[this->nb_SCCs];
// 	top_safety_scc(values, true);

// 	for (unsigned int scc_id = 0; scc_id < this->nb_SCCs; ++scc_id) {
// 		done[scc_id] = false;
// 		int x = this->SCCs[scc_id]->origin->getId();
// 		top_values[scc_id] = values[this->SCCs[scc_id]->origin->getId()];
// 	}

// 	top_dag(this->SCCs[this->initial->getTag()], done, top_values);
// 	return top_values[this->initial->getTag()];
// }


weight_t Automaton::top_LimInf (weight_t* top_values) const {
	weight_t values[this->states->size()];
	bool done[this->nb_SCCs];
	// top_safety_scc(values, false);
	top_safety_scc(values, true);

	for (unsigned int scc_id = 0; scc_id < this->nb_SCCs; ++scc_id) {
		done[scc_id] = false;
		top_values[scc_id] = this->min_domain;
	}

	for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
		if (this->states->at(state_id)->getTag() > -1) {
			top_values[this->states->at(state_id)->getTag()] = std::max(
							top_values[this->states->at(state_id)->getTag()],
							values[state_id]
					);
		}
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



weight_t Automaton::top_LimAvg_cycles (weight_t* top_values, SetList<Edge*>** scc_cycles, UltimatelyPeriodicWord** witness) const {
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
							weight_t value = distance[len-1][edge->getFrom()->getId()] - edge->getWeight()->getValue();
							if (distance[len][edge->getTo()->getId()] == infinity) {
								distance[len][edge->getTo()->getId()] = value;
								back_distance[len][edge->getTo()->getId()] = edge;
							}
							else {
								// distance[len][edge->getTo()->getId()] =
								// 		std::min(value, distance[len][edge->getTo()->getId()]);
								if (value < distance[len][edge->getTo()->getId()]) {
									distance[len][edge->getTo()->getId()] = value;
									back_distance[len][edge->getTo()->getId()] = edge;
								}
							}

							// weight_t old_value = distance[len][edge->getTo()->getId()];
							// weight_t new_value = distance[len-1][edge->getFrom()->getId()] - edge->getWeight()->getValue();
							// if (old_value == infinity || new_value < old_value) {
							// 	distance[len][edge->getTo()->getId()] = new_value;
							// 	back_distance[len][edge->getTo()->getId()] = edge;
							// }
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
		if (len_flag && top_values[this->states->at(state_id)->getTag()] <= min_lenght_avg) {
			top_values[this->states->at(state_id)->getTag()] = min_lenght_avg;
			scc_values[this->states->at(state_id)->getTag()] = min_lenght_avg;
			scc_back[this->states->at(state_id)->getTag()] = this->states->at(state_id);
		}
	}

	top_dag(this->SCCs[this->initial->getTag()], done, top_values);

	for (unsigned int scc_id = 0; scc_id < this->nb_SCCs; ++scc_id) {
		if (scc_values[scc_id] == top_values[scc_id]){
			
			bool spot[this->states->size()];
			
			for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
				spot[state_id] = false;
			}
			
			State* seek_state = scc_back[scc_id];
			int length = this->states->size();
			while(seek_state != nullptr && spot[seek_state->getId()] == false) {
				spot[seek_state->getId()] = true;
				seek_state = back_distance[length][seek_state->getId()]->getFrom();
				length--;
			}
			
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

	if (witness != nullptr) {
		constructWitness(LimInfAvg, witness, scc_values, top_values, scc_cycles, nullptr, nullptr);
	}

	return top_values[this->initial->getTag()];
}





bool Automaton::top_cycles_explore (State* target, State* state, bool* spot, weight_t (*filter)(weight_t,weight_t), weight_t* top_values, SetList<Edge*>** scc_cycles) const {
	if (spot[state->getId()] == true) return false;
	if (state == target) return true;
	
	spot[state->getId()] = true;
	for (Symbol* symbol : *(state->getAlphabet())) {
		for (Edge* edge : *(state->getSuccessors(symbol->getId()))) {
			if (edge->getFrom()->getTag() == edge->getTo()->getTag()) {
				weight_t value = filter(edge->getWeight()->getValue(), top_values[state->getTag()]);
				if (value == top_values[state->getTag()]) {
					if (top_cycles_explore(target, edge->getTo(), spot, filter, top_values, scc_cycles) == true) {
						scc_cycles[state->getTag()]->push(edge);
						return true;
					}
				}
			}
		}
	}
	return false;
}
void Automaton::top_cycles (weight_t (*filter)(weight_t,weight_t), weight_t* scc_values, weight_t* top_values, SetList<Edge*>** scc_cycles) const {
	bool spot[this->states->size()];
	
	for (unsigned int scc_id = 0 ; scc_id < this->nb_SCCs; ++scc_id) {
		scc_cycles[scc_id] = new SetList<Edge*>();
	}
	
	for (unsigned int state_id = 0 ; state_id < this->states->size(); ++state_id) {
		State* state = this->states->at(state_id);
		
		if (state->getTag() == -1) continue; // state not reachable
		if (scc_values[state->getId()] != top_values[state->getTag()]) continue; // top not held by SCC
		if (scc_cycles[state->getTag()]->size() != 0) continue; // cycle already computed
		
		for (Symbol* symbol : *(state->getAlphabet())) {
			for (Edge* edge : *(state->getSuccessors(symbol->getId()))) {
				if (edge->getFrom()->getTag() == edge->getTo()->getTag() && edge->getWeight()->getValue() == top_values[edge->getTo()->getTag()]) {
					for (unsigned int state_id = 0 ; state_id < this->states->size(); ++state_id) {
						spot[state_id] = false;
					}
					if (top_cycles_explore(edge->getFrom(), edge->getTo(), spot, filter, top_values, scc_cycles) == true) {
						scc_cycles[edge->getFrom()->getTag()]->push(edge);
					}
				}
			}
		}
	}

	return;
}


weight_t Automaton::top_LimInf_cycles (weight_t* top_values, SetList<Edge*>** scc_cycles, UltimatelyPeriodicWord** witness) const {
	weight_t (*filter)(weight_t,weight_t) = [] (weight_t x, weight_t y) -> weight_t {
		return std::min(x, y);
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

	if (witness != nullptr) {
		constructWitness(LimInf, witness, scc_values, top_values, scc_cycles, nullptr, nullptr);
		for (unsigned int scc_id = 0 ; scc_id < this->nb_SCCs; ++scc_id) {
			delete scc_cycles[scc_id];
		}
	}

	return top_values[this->initial->getTag()];
}


weight_t Automaton::top_LimSup_cycles (weight_t* top_values, SetList<Edge*>** scc_cycles, UltimatelyPeriodicWord** witness) const {
	weight_t (*filter)(weight_t,weight_t) = [] (weight_t x, weight_t y) -> weight_t {
		return std::max(x, y);
	};
	weight_t scc_values[this->states->size()];
	
	weight_t top = top_reachably(true, scc_values, top_values);
    top_cycles(filter, scc_values, top_values, scc_cycles);

    if (witness != nullptr) {
		constructWitness(LimSup, witness, scc_values, top_values, scc_cycles, nullptr, nullptr);
		for (unsigned int scc_id = 0 ; scc_id < this->nb_SCCs; ++scc_id) {
			delete scc_cycles[scc_id];
		}
	}

	return top;
}

bool Automaton::top_Sup_witness_explore (weight_t top, State* state, bool* spot, SetList<Edge*>* path) const {
	if (spot[state->getId()] == true) return false;
	
	spot[state->getId()] = true;
	for (Symbol* symbol : *(state->getAlphabet())) {
		for (Edge* edge : *(state->getSuccessors(symbol->getId()))) {
			if (edge->getWeight()->getValue() == top) {
				path->push(edge);
				return true;
			}
			else {
				weight_t value = std::max(edge->getWeight()->getValue(), top);
				if (value == top) {
					if (top_Sup_witness_explore(top, edge->getTo(), spot, path) == true) {
						path->push(edge);
						return true;
					}
				}
			}
		}
	}

	return false;
}

void Automaton::top_Sup_witness (weight_t top, SetList<Edge*>* path) const {
	bool spot[this->states->size()];
	
	for (unsigned int state_id = 0 ; state_id < this->states->size(); ++state_id) {
		spot[state_id] = false;
	}
	
	top_Sup_witness_explore(top, this->initial, spot, path);
}

weight_t Automaton::top_Sup_path (weight_t* top_values, SetList<Edge*>* path, UltimatelyPeriodicWord** witness) const {
	weight_t top = top_Sup(top_values);
	top_Sup_witness (top, path);
	constructWitness(Sup, witness, nullptr, top_values, nullptr, path, nullptr);
	return top;
}


int Automaton::top_Inf_witness_explore_post (weight_t top, State* state, bool* spot, bool* spot_back, SetList<Edge*>* witness_path ,SetList<Edge*>* witness_loop) const {
	if (spot[state->getId()] == true) {
		spot_back[state->getId()] = true;
		return 2;
	}
	
	spot[state->getId()] = true;
	for (Symbol* symbol : *(state->getAlphabet())) {
		for (Edge* edge : *(state->getSuccessors(symbol->getId()))) {
			weight_t value = std::min(edge->getWeight()->getValue(), top);
			if (value == top) {
				int tmp = top_Inf_witness_explore_post(top, edge->getTo(), spot, spot_back, witness_path, witness_loop);
				if (tmp == 1) {
					witness_path->push(edge);
					return 1;
				}
				if (tmp == 2) {
					if (spot_back[state->getId()] == true) {
						witness_loop->push(edge);
						return 1;
					}
					else {
						spot_back[state->getId()] = true;
						witness_loop->push(edge);
						return 2;
					}
				}
			}
		}
	}
	
	spot[state->getId()] = false;
	return 0;
}

bool Automaton::top_Inf_witness_post (State* init, weight_t top, SetList<Edge*>* witness_path, SetList<Edge*>* witness_loop) const {
	bool spot[this->states->size()];
	bool spot_back[this->states->size()];	
	
	for (unsigned int state_id = 0 ; state_id < this->states->size(); ++state_id) {
		spot[state_id] = false;
		spot_back[state_id] = false;
	}
	
	return (top_Inf_witness_explore_post (top, init, spot, spot_back, witness_path, witness_loop) != 0);
}

bool Automaton::top_Inf_witness_explore_pre (weight_t top, State* state, bool* spot, SetList<Edge*>* witness_path, SetList<Edge*>* witness_loop) const {
	if (spot[state->getId()] == true) return false;
	
	spot[state->getId()] = true;
	for (Symbol* symbol : *(state->getAlphabet())) {
		for (Edge* edge : *(state->getSuccessors(symbol->getId()))) {
			if (edge->getWeight()->getValue() == top) {
				if (top_Inf_witness_post (edge->getTo(), top, witness_path, witness_loop) == true) {
					witness_path->push(edge);
					return true;
				}
			}
			else {
				weight_t value = std::min(edge->getWeight()->getValue(), top);
				if (value == top) {
					if (top_Inf_witness_explore_pre(top, edge->getTo(), spot, witness_path, witness_loop) == true) {
						witness_path->push(edge);
						return true;
					}
				}
			}
		}
	}
	
	return false;
}

void Automaton::top_Inf_witness (weight_t top, SetList<Edge*>* witness_path, SetList<Edge*>* witness_loop) const {
	bool spot[this->states->size()];
	
	for (unsigned int state_id = 0 ; state_id < this->states->size(); ++state_id) {
		spot[state_id] = false;
	}
	
	top_Inf_witness_explore_pre (top, this->initial, spot, witness_path, witness_loop);
}

weight_t Automaton::top_Inf_path (weight_t* top_values, SetList<Edge*>* witness_path, SetList<Edge*>* witness_loop, UltimatelyPeriodicWord** witness) const {
	weight_t top = top_Inf(top_values);
	top_Inf_witness(top, witness_path, witness_loop);
	constructWitness(Inf, witness, nullptr, top_values, nullptr, witness_path, witness_loop);
	return top;
}

void Automaton::constructWitness(value_function_t f, UltimatelyPeriodicWord** witness, const weight_t* scc_values, const weight_t* top_values, SetList<Edge*>** scc_cycles, SetList<Edge*>* path, SetList<Edge*>* loop) const {
    (*witness) = new UltimatelyPeriodicWord{new Word(), new Word()};

	if (f == Sup) {
        for (Edge* edge : *path) {
            (*witness)->prefix->push_back(edge->getSymbol());
        }
		(*witness)->cycle->push_back(this->alphabet->at(0));
    }
	else if (f == Inf) {
		if (path->head()->getFrom()->getId() == path->back()->getTo()->getId()) {
			for (Edge* edge : *path) {
            	(*witness)->cycle->push_back(edge->getSymbol());
			}
        }
		else {
			for (Edge* edge : *path) {
				(*witness)->prefix->push_back(edge->getSymbol());
			}
			for (Edge* edge : *loop) {
				(*witness)->cycle->push_back(edge->getSymbol());
			}
		}
	}
	else { // prefix-independent value functions
		unsigned int top_state_id = 0;
		if (f == LimInf || f == LimSup) {
			for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
				if (scc_values[state_id] > scc_values[top_state_id]) {
					top_state_id = state_id;
				}
			}
		}
		else if (f == LimInfAvg || f == LimSupAvg) {
			int temp = 0;
			for (unsigned int scc_id = 0; scc_id < this->nb_SCCs; ++scc_id) {
				if (scc_values[scc_id] > scc_values[temp]) {
					temp = scc_id;
				}
			}

			for (unsigned int state_id = 0; state_id < this->states->size(); ++state_id) {
				if (this->states->at(state_id)->getTag() == temp) {
					top_state_id = state_id;
					break;
				}
			}
		}

		// construct a path to the start state of the top scc's cycle
		std::vector<Edge*> path_to_top_scc;
		State* target = (*scc_cycles[states->at(top_state_id)->getTag()]->begin())->getFrom();

		std::vector<State*> queue;
		std::vector<Edge*> predecessor_edge(this->states->size(), nullptr);
		std::vector<bool> visited(this->states->size(), false);

		queue.push_back(this->initial);
		visited[this->initial->getId()] = true;
		
		bool found = false;
		for (size_t i = 0; i < queue.size() && !found; i++) {
			State* current = queue[i];
			
			for (Symbol* symbol : *(current->getAlphabet())) {
				for (Edge* edge : *(current->getSuccessors(symbol->getId()))) {
					State* next = edge->getTo();
						
					// only consider states that lead to the top scc
					if (top_values[next->getTag()] == top_values[states->at(top_state_id)->getTag()]) {
						if (!visited[next->getId()]) {
							visited[next->getId()] = true;
							queue.push_back(next);
							predecessor_edge[next->getId()] = edge;

							if (next == target) {
								found = true;
								break;
							}
						}
					}
				}
				if (found) break;
			}
		}

		// reconstruct the path
		if (found) {
			State* current = target;
			while (current != this->initial) {
				Edge* edge = predecessor_edge[current->getId()];
				path_to_top_scc.insert(path_to_top_scc.begin(), edge);
				current = edge->getFrom();
			}
		}

		if (!scc_cycles[states->at(top_state_id)->getTag()]->empty()) {
			for (Edge* edge : path_to_top_scc) {
				(*witness)->prefix->push_back(edge->getSymbol());
			}
			for (Edge* edge : *scc_cycles[states->at(top_state_id)->getTag()]) {
				(*witness)->cycle->push_back(edge->getSymbol());
			}		
		}
	}
}

weight_t Automaton::compute_Top (value_function_t f, weight_t* top_values, UltimatelyPeriodicWord** witness) const {
	if (witness == nullptr) {
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
				QUAK_FAIL("automata top (without witness)");
		}
	}
	else {
		SetList<Edge*>* scc_cycles[this->nb_SCCs];
		SetList<Edge*>* path = new SetList<Edge*>();
		SetList<Edge*>* loop = new SetList<Edge*>();
        weight_t result;

        switch (f) {
            case Inf:
				result = top_Inf_path(top_values, path, loop, witness);
                break;
            case Sup:
                result = top_Sup_path(top_values, path, witness);
                break;
            case LimInf:
                result = top_LimInf_cycles(top_values, scc_cycles, witness);
                break;
            case LimSup:
                result = top_LimSup_cycles(top_values, scc_cycles, witness);
                break;
            case LimInfAvg: case LimSupAvg:
                result = top_LimAvg_cycles(top_values, scc_cycles, witness);
                break;
            default:
                QUAK_FAIL("automata top (with witness)");
        }

        delete path;
		delete loop;
        return result;
	}
}

value_function_t getValueFunctionDual (value_function_t f) {
	if (f == Inf) {
		return Sup;
	}
	else if (f == Sup) {
		return Inf;
	}
	else if (f == LimInf) {
		return LimSup;
	}
	else if (f == LimSup) {
		return LimInf;
	}
	else if (f == LimInfAvg) {
		return LimSupAvg;
	}
	else if (f == LimSupAvg) {
		return LimInfAvg;
	}
	else {
		std::cerr << "Unknown value function dual" << "\n";
    	abort();
	}
}

weight_t Automaton::compute_Bottom (value_function_t f, weight_t* bot_values, UltimatelyPeriodicWord** witness) {
	if (this->isDeterministic()) {
		invert_weights();
		value_function_t f_dual = getValueFunctionDual(f);
		weight_t bot = compute_Top(f_dual, bot_values, witness);
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
				found = this->isUniversal(f, x, witness);
			}

			return x;
		}
		else {
			QUAK_FAIL("automata bot");
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


weight_t Automaton::computeValue(value_function_t f, UltimatelyPeriodicWord* w) {
	if (f == Sup) {
		SetStd<State*> current_states;
		current_states.insert(this->initial);
		weight_t value = this->min_domain;

		for (unsigned int i = 0; i < w->prefix->getLength(); i++) {
			Symbol* symbol = w->prefix->at(i);
			SetStd<std::pair<State*, weight_t>> next_states_weights;
			
			for (State* state : current_states) {
				for (Edge* edge : *(state->getSuccessors(symbol->getId()))) {
					next_states_weights.insert(std::make_pair(edge->getTo(), edge->getWeight()->getValue()));
					value = std::max(value, edge->getWeight()->getValue());
				}
			}

			current_states.clear();
			for (const auto& pair : next_states_weights) {
				current_states.insert(pair.first);
			}
		}

		for (State* start_state : current_states) {
			SetStd<std::pair<State*, weight_t>> current_pairs;
			current_pairs.insert(std::make_pair(start_state, this->min_domain));
			
			for (unsigned int i = 0; i < w->cycle->getLength(); i++) {
				Symbol* symbol = w->cycle->at(i);
				SetStd<std::pair<State*, weight_t>> next_pairs;
				
				for (const auto& pair : current_pairs) {
					for (Edge* edge : *(pair.first->getSuccessors(symbol->getId()))) {
						value = std::max(value, edge->getWeight()->getValue());
						next_pairs.insert(std::make_pair(edge->getTo(), edge->getWeight()->getValue()));
					}
				}
				current_pairs = next_pairs;
			}
		}

		return value;
	}
	else if (f == Inf) {
		SetStd<State*> current_states;
		current_states.insert(this->initial);
		std::vector<weight_t> path_mins;
		
		SetStd<std::pair<State*, weight_t>> current_pairs;
		current_pairs.insert(std::make_pair(this->initial, this->max_domain));
		
		for (unsigned int i = 0; i < w->prefix->getLength(); i++) {
			Symbol* symbol = w->prefix->at(i);
			SetStd<std::pair<State*, weight_t>> next_pairs;
			
			for (const auto& pair : current_pairs) {
				for (Edge* edge : *(pair.first->getSuccessors(symbol->getId()))) {
					weight_t min_value = std::min(pair.second, edge->getWeight()->getValue());
					next_pairs.insert(std::make_pair(edge->getTo(), min_value));
				}
			}
			current_pairs = next_pairs;
		}
		
		weight_t best_value = this->min_domain;
		for (const auto& start_pair : current_pairs) {
			SetStd<std::pair<State*, weight_t>> cycle_pairs;
			cycle_pairs.insert(start_pair);
			
			for (unsigned int i = 0; i < w->cycle->getLength(); i++) {
				Symbol* symbol = w->cycle->at(i);
				SetStd<std::pair<State*, weight_t>> next_pairs;
				
				for (const auto& pair : cycle_pairs) {
					for (Edge* edge : *(pair.first->getSuccessors(symbol->getId()))) {
						weight_t min_value = std::min(pair.second, edge->getWeight()->getValue());
						next_pairs.insert(std::make_pair(edge->getTo(), min_value));
					}
				}
				cycle_pairs = next_pairs;
			}
			
			// Only consider values where we return to the start state
			for (const auto& pair : cycle_pairs) {
				if (pair.first == start_pair.first) {  // Check if we've returned to the starting state
					best_value = std::max(best_value, pair.second);
				}
			}
		}
		
		return best_value;
	}
	else if (f == LimSup) {
		SetStd<State*> current_states;
		current_states.insert(this->initial);
		
		for (unsigned int i = 0; i < w->prefix->getLength(); i++) {
			Symbol* symbol = w->prefix->at(i);
			SetStd<State*> next_states;
			
			for (State* state : current_states) {
				for (Edge* edge : *(state->getSuccessors(symbol->getId()))) {
					next_states.insert(edge->getTo());
				}
			}
			current_states = next_states;
		}

		weight_t value = this->min_domain;
    
		for (State* start_state : current_states) {
			SetStd<std::pair<State*, weight_t>> current_pairs;
			current_pairs.insert(std::make_pair(start_state, this->min_domain));
			
			for (unsigned int i = 0; i < w->cycle->getLength(); i++) {
				Symbol* symbol = w->cycle->at(i);
				SetStd<std::pair<State*, weight_t>> next_pairs;
				
				for (const auto& pair : current_pairs) {
					for (Edge* edge : *(pair.first->getSuccessors(symbol->getId()))) {
						next_pairs.insert(std::make_pair(
							edge->getTo(),
							std::max(pair.second, edge->getWeight()->getValue())
						));
					}
				}
				current_pairs = next_pairs;
			}
			
			for (const auto& pair : current_pairs) {
				if (pair.first == start_state) {
					value = std::max(value, pair.second);
				}
			}
		}
		
		return value;
	}
	else if (f == LimInf) {
		SetStd<State*> current_states;
		current_states.insert(this->initial);

		for (unsigned int i = 0; i < w->prefix->getLength(); i++) {
			Symbol* symbol = w->prefix->at(i);
			SetStd<State*> next_states;
			
			for (State* state : current_states) {
				for (Edge* edge : *(state->getSuccessors(symbol->getId()))) {
					next_states.insert(edge->getTo());
				}
			}
			current_states = next_states;
		}

		weight_t value = this->min_domain;

		for (State* start_state : current_states) {
			SetStd<std::pair<State*, weight_t>> current_pairs;
			current_pairs.insert(std::make_pair(start_state, this->max_domain));
			
			for (unsigned int i = 0; i < w->cycle->getLength(); i++) {
				Symbol* symbol = w->cycle->at(i);
				SetStd<std::pair<State*, weight_t>> next_pairs;
				
				for (const auto& pair : current_pairs) {
					for (Edge* edge : *(pair.first->getSuccessors(symbol->getId()))) {
						next_pairs.insert(std::make_pair(
							edge->getTo(),
							std::min(pair.second, edge->getWeight()->getValue())
						));
					}
				}
				current_pairs = next_pairs;
			}
			
			for (const auto& pair : current_pairs) {
				if (pair.first == start_state) {
					value = std::max(value, pair.second);
				}
			}
		}

		return value;
	}
	else if (f == LimInfAvg || f == LimSupAvg) {
		SetStd<State*> current_states;
		current_states.insert(this->initial);
		if (w->prefix != nullptr) {
			for (unsigned int i = 0; i < w->prefix->getLength(); i++) {
				Symbol* symbol = w->prefix->at(i);
				SetStd<State*> next_states;
				
				for (State* state : current_states) {
					for (Edge* edge : *(state->getSuccessors(symbol->getId()))) {
						next_states.insert(edge->getTo());
					}
				}
				current_states = next_states;
			}
		}

		weight_t value = this->min_domain;

		for (State* start_state : current_states) {
			SetStd<std::pair<State*, weight_t>> current_pairs;
			current_pairs.insert(std::make_pair(start_state, 0));
			
			for (unsigned int i = 0; i < w->cycle->getLength(); i++) {
				Symbol* symbol = w->cycle->at(i);
				SetStd<std::pair<State*, weight_t>> next_pairs;
				
				for (const auto& pair : current_pairs) {
					for (Edge* edge : *(pair.first->getSuccessors(symbol->getId()))) {
						next_pairs.insert(std::make_pair(
							edge->getTo(),
							pair.second + edge->getWeight()->getValue()
						));
					}
				}
				current_pairs = next_pairs;
			}
			
			for (const auto& pair : current_pairs) {
				if (pair.first == start_state) {
					weight_t avg = pair.second / (weight_t)w->cycle->getLength();
					value = std::max(value, avg);
				}
			}
		}

		return value;
	}
	else {
		QUAK_FAIL("compute value of lasso word");
	}
}

// -------------------------------- toStrings -------------------------------- //


void Automaton::print(bool full, bool bv_weights, bool bv_only) const {
	print(std::cout, full, bv_weights, bv_only);

}

void Automaton::print(std::ostream& out, bool full, bool bv_weights, bool bv_only) const {
	out << "automaton (" << this->name << "):\n";
	out << "\talphabet (" << this->alphabet->size() << "):";
	out << this->alphabet->toString(Symbol::toString) << "\n";
	out << "\tweights (" << this->weights->size() << "):";
	out << weights->toString(Weight::toString) << "\n";
	out << "\t\tMIN = " << std::to_string(min_domain) << "\n";
	out << "\t\tMAX = " << std::to_string(max_domain) << "\n";
	out << "\tstates (" << this->states->size() << "):";
	out << states->toString(State::toString) << "\n";
	out << "\t\tINITIAL = " << initial->getName() << "\n";
	out << "\tSCCs (" << this->nb_SCCs << "):";
	out << this->SCCs[this->initial->getTag()]->toString("\t\t") << "\n";
	unsigned int nb_edge = 0;
	for (unsigned int state_id = 0; state_id < states->size(); ++state_id) {
		for (Symbol* symbol : *(states->at(state_id)->getAlphabet())) {
			nb_edge += states->at(state_id)->getSuccessors(symbol->getId())->size();
		}
	}
	out << "\tedges (" << nb_edge << "):\n";
	for (unsigned int state_id = 0; state_id < states->size(); ++state_id) {
		for (Symbol* symbol : *(states->at(state_id)->getAlphabet())) {
            auto *state = states->at(state_id);
            for (auto *edge : *state->getSuccessors(symbol->getId())) {
                out << "\t\t" << edge->getSymbol()->toString() << " : ";
                if (bv_only) {
                    out << "0x" << std::hex << edge->getWeight()->getValue().to_bv();
                } else {
                    if (full) {
                      out << std::setprecision(std::numeric_limits<weight_t::T>::max_digits10)
                                << std::fixed;
                    }
                    out << *edge->getWeight()->getValue();
                    if (bv_weights) {
                        out << " (" << "0x" << std::hex << edge->getWeight()->getValue().to_bv() << ")";
                    }
                }
                out << ", " << edge->getFrom()->getName() << " -> "
                          << edge->getTo()->getName() << "\n";
            }
		}
	}
	out << "\n";
}
