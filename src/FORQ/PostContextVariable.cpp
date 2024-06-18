
#include "PostContextVariable.h"


PostContextVariable::~PostContextVariable () {
	for (std::pair<State*,SetStd<std::pair<ContextOf*,std::pair<Word*,weight_t>>>*> pairmap : *this) {
		for (std::pair<ContextOf*,std::pair<Word*,weight_t>> pairset : *(pairmap.second)) {
		pairset.first->decreaseRef();
			if (pairset.first->getRef() == 0) {
				delete pairset.first;
				delete pairset.second.first;
			}
		}
	}
}


PostContextVariable::PostContextVariable () :
		MapStd<State*,SetStd<std::pair<ContextOf*,std::pair<Word*,weight_t>>>*>()
	{}



SetStd<std::pair<ContextOf*,std::pair<Word*,weight_t>>>* PostContextVariable::getSetOfContextsOrNULL (State* stateA) {
	if (this->contains(stateA) == false) {
		return NULL;
	}
	else {
		return this->at(stateA);
	}
}



void PostContextVariable::add (State* stateA, ContextOf* setB, Word* word, weight_t value) {
	if (this->contains(stateA) == false) {
		this->insert(stateA, new SetStd<std::pair<ContextOf*, std::pair<Word*,weight_t>>>());
		this->at(stateA)->insert(std::pair<ContextOf*, std::pair<Word*,weight_t>>(setB, std::pair<Word*,weight_t>(word,value)));
		setB->increaseRef();
	}
	else {
		unsigned int n = this->at(stateA)->size();
		this->at(stateA)->insert(std::pair<ContextOf*, std::pair<Word*,weight_t>>(setB, std::pair<Word*,weight_t>(word,value)));
		if (n < this->at(stateA)->size()) setB->increaseRef();
	}
}



void PostContextVariable::erase (State* stateA, std::pair<ContextOf*, std::pair<Word*,weight_t>> pair) {
	if (this->contains(stateA) == true) {
		unsigned int n = this->at(stateA)->size();
		this->at(stateA)->erase(pair);
		if (n < this->at(stateA)->size()) {
			pair.first->decreaseRef();
			if(pair.first->getRef() == 0) {
				delete pair.first;
				delete pair.second.first;
			}
		}
	}
}



void PostContextVariable::clear () {
	for (std::pair<State*,SetStd<std::pair<ContextOf*,std::pair<Word*,weight_t>>>*> pairmap : *this) {
		for (std::pair<ContextOf*,std::pair<Word*,weight_t>> pairset : *(pairmap.second)) {
			pairset.first->decreaseRef();
			if (pairset.first->getRef() == 0) {
				delete pairset.first;
				delete pairset.second.first;
			}
		}
	}
	all.clear();
}



bool PostContextVariable::addIfMin (State* stateA, ContextOf* setB, Word* word, weight_t value) {
	if (this->contains(stateA) == false)
		this->insert(stateA, new SetStd<std::pair<ContextOf*, std::pair<Word*,weight_t>>>());

	auto iter = this->at(stateA)->begin();
	while (iter != this->at(stateA)->end()) {
		if (iter->first->smaller_than(setB) == true) return false;
		if (setB->smaller_than(iter->first) == true) {
			auto tmp = iter;
			++iter;
			this->at(stateA)->erase(*tmp);
		}
		else {
			++iter;
		}
	}

	this->add(stateA, setB, word, value);
	return true;
}



bool PostContextVariable::addIfMax (State* stateA, ContextOf* setB, Word* word, weight_t value) {
	if (this->contains(stateA) == false)
		this->insert(stateA, new SetStd<std::pair<ContextOf*, std::pair<Word*,weight_t>>>());

	auto iter = this->at(stateA)->begin();
	while (iter != this->at(stateA)->end()) {
		if (setB->smaller_than(iter->first) == true) return false;
		if (iter->first->smaller_than(setB) == true) {
			auto tmp = iter;
			++iter;
			this->at(stateA)->erase(*tmp);
		}
		else {
			++iter;
		}
	}

	this->add(stateA, setB, word, value);
	return true;
}


