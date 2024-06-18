
#include "FixpointStem.h"


FixpointStem::~FixpointStem () {
	delete this->buffer;
	delete this->content;
	delete this->updates;
}

FixpointStem::FixpointStem (State* initA, State* initB, bool rev) {
	this->reversed_inclusion = rev;
	this->content = new PostTargetVariable();
	this->updates = new PostTargetVariable();
	this->buffer = new PostTargetVariable();

	TargetOf* init_setB = new TargetOf();
	Word* init_word = new Word();
	init_setB->add(initB);
	this->updates->add(initA, init_setB, init_word);
	this->content->add(initA, init_setB, init_word);
}


SetStd<std::pair<TargetOf*,Word*>>* FixpointStem::getSetOfTargetsOrNULL (State* stateA) {
	return this->content->getSetOfTargetsOrNULL(stateA);
}


bool FixpointStem::addIfExtreme (State* stateA, TargetOf* setB, Word* word) {
	if (this->reversed_inclusion) {
		return this->content->addIfMax(stateA, setB, word);
	}
	else {
		return this->content->addIfMin(stateA, setB, word);
	}
}


TargetOf* FixpointStem::post (TargetOf* currentB, Symbol* symbol) {
	TargetOf* postB = new TargetOf();
	for (State* state: *currentB) {
		for (Edge* edge : *(state->getSuccessors(symbol->getId())))  {
			postB->add(edge->getTo());
		}
	}
	return postB;
}


bool FixpointStem::apply () {
	for (std::pair<State*, SetStd<std::pair<TargetOf*,Word*>>*> mapfromA : *(this->updates)) {
		for (Symbol* symbol : *(mapfromA.first->getAlphabet())) {
			auto iter = mapfromA.second->begin();
			while (iter != mapfromA.second->end()) {
				TargetOf* postB = post(iter->first, symbol);
				Word* word = new Word(iter->second, symbol);
				++iter;

				for (Edge* edgeA : *(mapfromA.first->getSuccessors(symbol->getId()))) {
					if(addIfExtreme(edgeA->getTo(), postB, word)) {
						buffer->add(edgeA->getTo(), postB, word);
					}
				}

				if (postB->getRef() == 0) {
					delete word;
					delete postB;
				}
			}
		}
	}

	PostTargetVariable* tmp;
	tmp = this->updates;
	this->updates = this->buffer;
	this->buffer = tmp;
	this->buffer->clear();

	return (this->updates->size() > 0);
}


