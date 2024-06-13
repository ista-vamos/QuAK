
#include "TargetOf.h"
#include <algorithm>


TargetOf::~TargetOf() {}


TargetOf::TargetOf() : SetStd<State*>() {}


void TargetOf::add (State* state) {
	insert(state);
}


bool TargetOf::smaller_than (TargetOf* other){
	return std::includes(other->all.begin(), other->all.end(), this->all.begin(), this->all.end());
}

