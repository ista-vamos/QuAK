
#include "TargetOf.h"
#include <algorithm>


TargetOf::~TargetOf() {}


TargetOf::TargetOf() : SetStd<State*>() {}


void TargetOf::add (State* stateB) {
	insert(stateB);
}


bool TargetOf::smaller_than (TargetOf* other){
	return std::includes(other->begin(), other->end(), this->begin(), this->end());
}


/*
std::string TargetOf::TargetOf::toString (TargetOf* tmp) {
	return tmp->toString();
}
std::string TargetOf::toString () const {
	return SetStd<State*>::toString(State::toString);
}
*/
