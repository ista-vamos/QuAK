
#include "TargetOf.h"
#include <algorithm>


TargetOf::~TargetOf() {}



TargetOf::TargetOf() : SetStd<State*>() {}



TargetOf::TargetOf(TargetOf* currentB, Symbol* symbol) : SetStd<State*>() {
	for (State* state: *currentB) {
		for (Edge* edge : *(state->getSuccessors(symbol->getId())))  {
			add(edge->getTo());
		}
	}
}




void TargetOf::add (State* stateB) {
	insert(stateB);
}



bool TargetOf::smaller_than (TargetOf* other){
	return std::includes(other->begin(), other->end(), this->begin(), this->end());
}


