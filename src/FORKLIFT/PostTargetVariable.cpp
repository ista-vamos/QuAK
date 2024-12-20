
#include "PostTargetVariable.h"


PostTargetVariable::~PostTargetVariable () {
	for (std::pair<State*,SetStd<std::pair<TargetOf*,Word*>>*> pairmap : *this) {
		for (std::pair<TargetOf*,Word*> pairset : *(pairmap.second)) {
			pairset.first->decreaseRef();
			if (pairset.first->getRef() == 0) {
				delete pairset.first;
				delete pairset.second;
			}
		}
		delete pairmap.second;
	}
}


PostTargetVariable::PostTargetVariable () :
		MapStd<State*,SetStd<std::pair<TargetOf*,Word*>>*>()
	{}


SetStd<std::pair<TargetOf*,Word*>>* PostTargetVariable::getSetOfTargetsOrNULL (State* stateA) {
	if (this->contains(stateA) == false) {
		return nullptr;
	}
	else {
		return this->at(stateA);
	}
}



void PostTargetVariable::add (State* stateA, TargetOf* setB, Word* w) {
	if (this->contains(stateA) == false) {
		this->insert(stateA, new SetStd<std::pair<TargetOf*, Word*>>());
		this->at(stateA)->insert(std::pair<TargetOf*, Word*>(setB, w));
		setB->increaseRef();
	}
	else {
		unsigned int n = this->at(stateA)->size();
		this->at(stateA)->insert(std::pair<TargetOf*, Word*>(setB, w));
		if (n < this->at(stateA)->size()) setB->increaseRef();
	}
}



void PostTargetVariable::erase (State* stateA, std::pair<TargetOf*, Word*> pair) {
	if (this->contains(stateA) == true) {
		unsigned int n = this->at(stateA)->size();
		this->at(stateA)->erase(pair);
		if (n > this->at(stateA)->size()) {
			pair.first->decreaseRef();
			if(pair.first->getRef() == 0) {
				delete pair.first;
				delete pair.second;
			}
		}
	}
}


void PostTargetVariable::clear () {
	for (std::pair<State*,SetStd<std::pair<TargetOf*,Word*>>*> pairmap : *this) {
		for (std::pair<TargetOf*,Word*> pairset : *(pairmap.second)) {
			pairset.first->decreaseRef();
			if (pairset.first->getRef() == 0) {
				delete pairset.first;
				delete pairset.second;
			}
		}
		delete pairmap.second;
	}

	MapStd<State*,SetStd<std::pair<TargetOf*,Word*>>*>::clear();
}



bool PostTargetVariable::addIfMin (State* stateA, TargetOf* setB, Word* word) {
	if (this->contains(stateA) == false)
		this->insert(stateA, new SetStd<std::pair<TargetOf*, Word*>>());


	auto iter = this->at(stateA)->begin();
	while (iter!= this->at(stateA)->end()) {
		if (iter->first->smaller_than(setB) == true) return false;
		if (setB->smaller_than(iter->first) == true) {
			auto tmp = iter;
			++iter;
			this->erase(stateA, *tmp);
		}
		else{
			++iter;
		}
	}

	this->add(stateA, setB, word);
	return true;
}




bool PostTargetVariable::addIfMax (State* stateA, TargetOf* setB, Word* word) {
	if (this->contains(stateA) == false)
		this->insert(stateA, new SetStd<std::pair<TargetOf*, Word*>>());

	auto iter = this->at(stateA)->begin();
	while (iter!= this->at(stateA)->end()) {
		if (setB->smaller_than(iter->first) == true) return false;
		if (iter->first->smaller_than(setB) == true) {
			auto tmp = iter;
			++iter;
			this->erase(stateA, *tmp);
		}
		else{
			++iter;
		}
	}

	this->add(stateA, setB, word);
	return true;
}




