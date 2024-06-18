
#include "StateRelation.h"

StateRelation::~StateRelation() {
	for (std::pair<State*, TargetOf*> pair : *this) {
		delete pair.second;
	}
}



StateRelation::StateRelation () : MapStd<State*,TargetOf*>() {}



void StateRelation::add (State* fromB, State* toB) {
	if (this->contains(fromB) == false)
		this->insert(fromB, new TargetOf());
	this->at(fromB)->add(toB);
}



bool StateRelation::smaller_than (StateRelation* other){
	for (std::pair<State*, TargetOf*> pair : *this) {
		if (other->contains(pair.first) == false) return false;
		if (pair.second->smaller_than(other->at(pair.first))) return false;
	}
	return true;
}

