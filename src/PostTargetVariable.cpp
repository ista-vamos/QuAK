
#include "PostTargetVariable.h"



PostTargetVariable::PostTargetVariable () :
		MapStd<State*,SetStd<std::pair<TargetOf*,Word*>>*>()
	{}



void PostTargetVariable::add (State* stateB, TargetOf* new_target, Word* w) {
	if (this->contains(stateB) == false)
		this->insert(stateB, new SetStd<std::pair<TargetOf*, Word*>>());
	this->at(stateB)->insert(std::pair<TargetOf*, Word*>(new_target, w));
}


bool PostTargetVariable::addIfMin (State* stateB, TargetOf* new_target, Word* w) {
	if (this->contains(stateB) == false) {
		this->at(stateB)->insert(std::pair<TargetOf*, Word*>(new_target, w));
		return true;
	}

	for (std::pair<TargetOf*, Word*> pair : *(this->at(stateB))) {
		TargetOf* target = pair.first;
		if (target->smaller_than(new_target) == true) return false;
		if (new_target->smaller_than(target) == true) this->at(stateB)->erase(pair);
	}

	this->add(stateB, new_target, w);
	return true;
}




bool PostTargetVariable::addIfMax (State* stateB, TargetOf* new_target, Word* w) {
	if (this->contains(stateB) == false) {
		this->at(stateB)->insert(std::pair<TargetOf*, Word*>(new_target, w));
		return true;
	}

	for (std::pair<TargetOf*, Word*> pair : *(this->at(stateB))) {
		TargetOf* target = pair.first;
		if (new_target->smaller_than(target) == true) return false;
		if (target->smaller_than(new_target) == true) this->at(stateB)->erase(pair);
	}

	this->add(stateB, new_target, w);
	return true;
}




