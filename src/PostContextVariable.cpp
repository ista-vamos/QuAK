
#include "PostContextVariable.h"



PostContextVariable::PostContextVariable () :
		MapStd<State*,SetStd<std::pair<ContextOf*,Word*>>*>()
	{}



void PostContextVariable::add (State* stateB, ContextOf* new_context, Word* w) {
	if (this->contains(stateB) == false)
		this->insert(stateB, new SetStd<std::pair<ContextOf*, Word*>>());
	this->at(stateB)->insert(std::pair<ContextOf*, Word*>(new_context, w));
}


bool PostContextVariable::addIfMin (State* stateB, ContextOf* new_context, Word* w) {
	if (this->contains(stateB) == false) {
		this->at(stateB)->insert(std::pair<ContextOf*, Word*>(new_context, w));
		return true;
	}

	for (std::pair<ContextOf*, Word*> pair : *(this->at(stateB))) {
		if (pair.first->smaller_than(new_context) == true) return false;
		if (new_context->smaller_than(pair.first) == true) this->at(stateB)->erase(pair);
	}

	this->add(stateB, new_context, w);
	return true;
}




bool PostContextVariable::addIfMax (State* stateB, ContextOf* new_context, Word* w) {
	if (this->contains(stateB) == false) {
		this->at(stateB)->insert(std::pair<ContextOf*, Word*>(new_context, w));
		return true;
	}

	for (std::pair<ContextOf*, Word*> pair : *(this->at(stateB))) {
		if (new_context->smaller_than(pair.first) == true) return false;
		if (pair.first->smaller_than(new_context) == true) this->at(stateB)->erase(pair);
	}

	this->add(stateB, new_context, w);
	return true;
}




