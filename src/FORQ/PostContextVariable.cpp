
#include "PostContextVariable.h"


PostContextVariable::~PostContextVariable () {
	for (std::pair<State*,SetStd<std::pair<ContextOf*,Word*>>*> pairmap : *this) {
		for (std::pair<ContextOf*,Word*> pairset : *(pairmap.second)) {
			delete pairset.first;
			delete pairset.second;
		}
	}
}


PostContextVariable::PostContextVariable () :
		MapStd<State*,SetStd<std::pair<ContextOf*,Word*>>*>()
	{}



void PostContextVariable::add (State* stateA, ContextOf* setB, Word* w) {
	if (this->contains(stateA) == false)
		this->insert(stateA, new SetStd<std::pair<ContextOf*, Word*>>());
	this->at(stateA)->insert(std::pair<ContextOf*, Word*>(setB, w));
}


bool PostContextVariable::addIfMin (State* stateA, ContextOf* setB, Word* w) {
	if (this->contains(stateA) == false) {
		this->at(stateA)->insert(std::pair<ContextOf*, Word*>(setB, w));
		return true;
	}

	for (std::pair<ContextOf*, Word*> pair : *(this->at(stateA))) {
		if (pair.first->smaller_than(setB) == true) return false;
		if (setB->smaller_than(pair.first) == true) {
			this->at(stateA)->erase(pair);
			delete pair.first;
			delete pair.second;
		}
	}

	this->add(stateA, setB, w);
	return true;
}




bool PostContextVariable::addIfMax (State* stateA, ContextOf* setB, Word* w) {
	if (this->contains(stateA) == false) {
		this->at(stateA)->insert(std::pair<ContextOf*, Word*>(setB, w));
		return true;
	}

	for (std::pair<ContextOf*, Word*> pair : *(this->at(stateA))) {
		if (setB->smaller_than(pair.first) == true) return false;
		if (pair.first->smaller_than(setB) == true) {
			this->at(stateA)->erase(pair);
			delete pair.first;
			delete pair.second;
		}
	}

	this->add(stateA, setB, w);
	return true;
}




