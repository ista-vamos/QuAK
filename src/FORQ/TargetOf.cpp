
#include "TargetOf.h"
#include <algorithm>


TargetOf::~TargetOf() {}


TargetOf::TargetOf() : SetStd<State*>() {}


void TargetOf::add (State* stateB) {
	insert(stateB);
}


bool TargetOf::smaller_than (TargetOf* other){
	return std::includes(other->all.begin(), other->all.end(), this->all.begin(), this->all.end());
}


/*std::string TargetOf::toString () const {
	return SetStd<State*>::toString(State::toString);
}*/
