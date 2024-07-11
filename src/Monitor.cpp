
#include "Monitor.h"
#include "utility.h"


Monitor::Monitor(std::string filename, value_function_t f) : Automaton(filename, nullptr) {
	this->current = this->initial;
	this->top_values = new weight_t[this->nb_SCCs];
	this->bot_values = new weight_t[this->nb_SCCs];
	compute_Top(f, top_values);
	compute_Bottom(f, bot_values);
}


Monitor::Monitor(const Automaton* A, value_function_t f) : Automaton(A, f) {
	this->current = this->initial;
	this->top_values = new weight_t[this->nb_SCCs];
	this->bot_values = new weight_t[this->nb_SCCs];
	compute_Top(f, top_values);
	compute_Bottom(f, bot_values);
}


void Monitor::read(Symbol* symbol) {
	if (this->current->getAlphabet()->contains(symbol) == false)
		fail("incomplete monitor");

	if (this->current->getSuccessors(symbol->getId())->size() != 1)
		fail("non-deterministic monitor");

	Edge* edge = *(this->current->getSuccessors(symbol->getId())->begin());
	this->current = edge->getTo();
}


weight_t Monitor::getLowest() const {
	return this->bot_values[this->current->getTag()];
}


weight_t Monitor::getHighest() const {
	return this->top_values[this->current->getTag()];
}