
#include "ContextOf.h"

ContextOf::~ContextOf() {
	for (unsigned int weight_id = 0; weight_id < this->size(); ++weight_id) {
		delete this->at(weight_id);
	}
}


ContextOf::ContextOf(unsigned int capacity) : MapVec<StateRelation*>(capacity) {
	for (unsigned int weight_id = 0; weight_id < this->size(); ++weight_id) {
		this->insert(weight_id, new StateRelation());
	}
}


void ContextOf::add (State* fromB, State* toB, unsigned int weight_id) {
	// fixme: redundancy
	for (unsigned int id = weight_id; weight_id >= 0; --id) {
		this->at(id)->add(fromB, toB);
	}
}

StateRelation* ContextOf::at (unsigned int weight_id) const {
	return this->at(weight_id);
}


bool ContextOf::smaller_than (ContextOf* other) {
	for (unsigned int weight_id = 0; weight_id < this->size(); ++weight_id) {
		if (this->at(weight_id)->smaller_than(other->at(weight_id)) == false) return false;
	}
	return true;
}
