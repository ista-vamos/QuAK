
#include "FixpointStem.h"
#include "../utility.h"

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



bool FixpointStem::apply () {
	auto iterA = this->updates->begin();
	for (; iterA != this->updates->end(); ++iterA) {
		auto iter_symbol = iterA->first->getAlphabet()->begin();
		for (; iter_symbol != iterA->first->getAlphabet()->end(); ++iter_symbol) {
			auto iterB = iterA->second->begin();
			for (; iterB != iterA->second->end(); ++iterB) {
				TargetOf* postB = new TargetOf(iterB->first, *iter_symbol);
				Word* word = new Word(iterB->second, *iter_symbol);

				for (Edge* edgeA : *(iterA->first->getSuccessors((*iter_symbol)->getId()))) {
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


