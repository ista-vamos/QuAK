
#include "ContextOf.h"

ContextOf::~ContextOf() {
	for (unsigned int weight_id = 0; weight_id < this->size(); ++weight_id) {
		delete this->at(weight_id);
	}
}



ContextOf::ContextOf(unsigned int capacity) : MapArray<StateRelation*>(capacity) {
	for (unsigned int weight_id = 0; weight_id < this->size(); ++weight_id) {
		this->insert(weight_id, new StateRelation());
	}
}


/*
ContextOf::ContextOf (ContextOf* currentB, Symbol* symbol) : MapArray<StateRelation*>(currentB->size()) {
	for (unsigned int weight_id = 0; weight_id < this->size(); ++weight_id) {
		for (std::pair<State*,TargetOf*> pairB : *(currentB->at(weight_id))) {
			for (State* fromB: *(pairB.second)) {
				for (Edge* edgeB : *(fromB->getSuccessors(symbol->getId()))) {
					// fixme: consider only edge within the current SCC (only if relevance applied)
					weight_t max_weight_id = std::max(weight_id, edgeB->getWeight()->getId());
					add(pairB.first, edgeB->getTo(), max_weight_id);
					// -- since weight are sorted
					// -- id_1 < id_2 <==>  value_1 < value_2
				}
			}
		}
	}
}
*/



void ContextOf::add (State* fromB, State* toB, unsigned int weight_id) {
	// fixme: redundancy
	for (unsigned int id = 0; id <= weight_id; ++id) {
		this->at(id)->add(fromB, toB);
	}
}



bool ContextOf::smaller_than (ContextOf* other) {
	for (unsigned int weight_id = 0; weight_id < this->size(); ++weight_id) {
		if (this->at(weight_id)->smaller_than(other->at(weight_id)) == false) {
			return false;
		}
	}
	return true;
}
