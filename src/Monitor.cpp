#include <iostream>

#include "Monitor.h"
#include "utility.h"


Monitor::Monitor(const std::string &filename, value_function_t f)
:
  automaton_mem(new Automaton(filename, nullptr)),
  monitor(createMonitor(automaton_mem.get(), f)), value_fun(f) {

  initializeSymbols();
}

Monitor::Monitor(const Automaton* A, value_function_t f)
: monitor(createMonitor(A, f)), value_fun(f) {

  initializeSymbols();
}

MonitorImpl *Monitor::createMonitor(const Automaton *A, value_function_t f) {
  switch(f) {
    case Avg: return new MonitorAvg(A);
  }

  assert(false && "Unhandled monitor type");
  abort();
  return nullptr;
}

void Monitor::initializeSymbols() {
  const auto *A = monitor->getAutomaton();
  auto *alphabet = A->getAlphabet();
  for (auto *symbol : *alphabet) {
    assert(symbols.count(symbol->getName()) == 0
           && "Two symbols with the same name");
    symbols[symbol->getName()] = symbol;
  }
}


weight_t Monitor::next(const std::string& a) {
  auto it = symbols.find(a);
  if (it == symbols.end()) {
    std::cerr << "Unknown symbol: '" << a << "'\n";
    abort();
  }

  return monitor->next(it->second);
}

MonitorImpl::MonitorImpl(const Automaton *A) : automaton(A) {}

weight_t MonitorAvg::next(Symbol *s) {
  auto *succs = state->getSuccessors(s->getId());
  assert(succs && "No successors");
  assert(succs->size() == 1
         && "Non-deterministic automata not supported yet");

  auto *edge = (*succs->begin());

  state = edge->getTo();
  sum += edge->getWeight()->getValue();

  return sum / ++N;
}






#if 0
MonitorMinMax::MonitorMinMax(std::string filename, value_function_t f)
  : Automaton(filename, nullptr) {
	this->state = this->initial;
	this->top_values = new weight_t[this->nb_SCCs];
	this->bot_values = new weight_t[this->nb_SCCs];
	compute_Top(f, top_values);
	compute_Bottom(f, bot_values);
}


MonitorMinMax::MonitorMinMax(const Automaton* A, value_function_t f) : Automaton(A, f) {
	this->state = this->initial;
	this->top_values = new weight_t[this->nb_SCCs];
	this->bot_values = new weight_t[this->nb_SCCs];
	compute_Top(f, top_values);
	compute_Bottom(f, bot_values);
}


void MonitorMinMax::next(Symbol* symbol) {
	if (this->state->getAlphabet()->contains(symbol) == false)
		fail("incomplete monitor");

	if (this->state->getSuccessors(symbol->getId())->size() != 1)
		fail("non-deterministic monitor");

	Edge* edge = *(this->state->getSuccessors(symbol->getId())->begin());
	this->state = edge->getTo();
}


weight_t MonitorMinMax::getLowest() const {
	return this->bot_values[this->state->getTag()];
}


weight_t MonitorMinMax::getHighest() const {
	return this->top_values[this->state->getTag()];
}
#endif
