
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
