
#include "ContextOf.h"

ContextOf::ContextOf(unsigned int capacity) : MapVec<StateRelation*>(capacity) {
	for (unsigned int weight_id = 0; weight_id < this->size(); ++weight_id) {
		this->insert(weight_id, new StateRelation());
	}
}


void ContextOf::add (State* fromB, State* toB, Weight<weight_t>* weight) {
	// fixme: redundancy
	for (unsigned int weight_id = weight->getId(); weight_id >= 0; --weight_id) {
		this->at(weight_id)->add(fromB, toB);
	}
}


bool ContextOf::smaller_than (ContextOf* other) {
	for (unsigned int weight_id = 0; weight_id < this->size(); ++weight_id) {
		if (this->at(weight_id)->smaller_than(other->at(weight_id)) == false) return false;
	}
	return true;
}
