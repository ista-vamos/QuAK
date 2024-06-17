
#include "PostTargetVariable.h"

PostTargetVariable::~PostTargetVariable () {
	for (std::pair<State*,SetStd<std::pair<TargetOf*,Word*>>*> pairmap : *this) {
		for (std::pair<TargetOf*,Word*> pairset : *(pairmap.second)) {
			delete pairset.first;
			delete pairset.second;
		}
	}
}


PostTargetVariable::PostTargetVariable () :
		MapStd<State*,SetStd<std::pair<TargetOf*,Word*>>*>()
	{}


SetStd<std::pair<TargetOf*,Word*>>* PostTargetVariable::getSetOfTargets (State* stateA) {
	return this->at(stateA);
}



void PostTargetVariable::add (State* stateA, TargetOf* setB, Word* w) {
	if (this->contains(stateA) == false)
		this->insert(stateA, new SetStd<std::pair<TargetOf*, Word*>>());
	this->at(stateA)->insert(std::pair<TargetOf*, Word*>(setB, w));
}



bool PostTargetVariable::addIfMin (State* stateA, TargetOf* setB, Word* w) {
	if (this->contains(stateA) == false) {
		this->insert(stateA, new SetStd<std::pair<TargetOf*, Word*>>());
		this->at(stateA)->insert(std::pair<TargetOf*, Word*>(setB, w));
		return true;
	}

	for (std::pair<TargetOf*, Word*> pair : *(this->at(stateA))) {
		TargetOf* target = pair.first;
		if (target->smaller_than(setB) == true) return false;
		if (setB->smaller_than(target) == true) {
			delete pair.first;
			delete pair.second;
			this->at(stateA)->erase(pair);
		}
	}

	this->add(stateA, setB, w);
	return true;
}




bool PostTargetVariable::addIfMax (State* stateA, TargetOf* setB, Word* w) {
	if (this->contains(stateA) == false) {
		this->insert(stateA, new SetStd<std::pair<TargetOf*, Word*>>());
		this->at(stateA)->insert(std::pair<TargetOf*, Word*>(setB, w));
		return true;
	}

	for (std::pair<TargetOf*, Word*> pair : *(this->at(stateA))) {
		TargetOf* target = pair.first;
		if (setB->smaller_than(target) == true) return false;
		if (target->smaller_than(setB) == true) {
			delete pair.first;
			delete pair.second;
			this->at(stateA)->erase(pair);
		}
	}

	this->add(stateA, setB, w);
	return true;
}




