
#include "FixpointLoop.h"

FixpointLoop::~FixpointLoop () {
	delete this->buffer;
	delete this->content;
	delete this->updates;
}


FixpointLoop::FixpointLoop (State* initA, TargetOf* initB, unsigned int capacity) {
	this->capacity = capacity;
	this->content = new PostContextVariable();
	this->updates = new PostContextVariable();
	this->buffer = new PostContextVariable();


	for (Symbol* symbol : *(initA->getAlphabet())) {
		ContextOf* init_setB = new ContextOf(this->capacity);

		for (State* fromB : *initB) {
			for (Edge* edgeB : *(fromB->getSuccessors(symbol->getId()))) {
				init_setB->add(fromB, edgeB->getTo(), edgeB->getWeight()->getId());
			}
		}

		for (Edge* edgeA : *(initA->getSuccessors(symbol->getId()))) {
			Word* init_word = new Word(symbol);
			this->content->add(edgeA->getTo(), init_setB, init_word, edgeA->getWeight()->getValue());
			this->updates->add(edgeA->getTo(), init_setB, init_word, edgeA->getWeight()->getValue());
		}
	}
}



SetStd<std::pair<ContextOf*,std::pair<Word*,weight_t>>>* FixpointLoop::getSetOfContextsOrNULL (State* stateA) {
	return this->content->getSetOfContextsOrNULL(stateA);
}



bool FixpointLoop::addIfExtreme (State* stateA, ContextOf* setB, Word* word, weight_t value) {
	return this->content->addIfMin(stateA, setB, word, value);
}



ContextOf* FixpointLoop::post (ContextOf* currentB, Symbol* symbol) {
	ContextOf* postB = new ContextOf(this->capacity);
	for (unsigned int weight_id = 0; weight_id < this->capacity; ++weight_id) {
		for (std::pair<State*,TargetOf*> pairB : *(currentB->at(weight_id))) {
			for (State* fromB: *(pairB.second)) {
				for (Edge* edgeB : *(fromB->getSuccessors(symbol->getId()))) {
					weight_t max_weight_id = std::max(weight_id, edgeB->getWeight()->getId());
					postB->add(pairB.first, edgeB->getTo(), max_weight_id);
					// -- since weight are sorted
					// -- weight_id_1 < weight_id_2 <==>  weight_1 < weight_2
				}
			}
		}
	}
	return postB;
}



bool FixpointLoop::apply () {
	for (std::pair<State*, SetStd<std::pair<ContextOf*,std::pair<Word*,weight_t>>>*> mapfromA : *(this->updates)) {
		for (Symbol* symbol : *(mapfromA.first->getAlphabet())) {
			auto iter = mapfromA.second->begin();
			while (iter != mapfromA.second->end()) {
				ContextOf* postB = post(iter->first, symbol);
				Word* word = new Word(iter->second.first, symbol);
				weight_t value = iter->second.second;
				++iter;

				for (Edge* edgeA : *(mapfromA.first->getSuccessors(symbol->getId()))) {
					if (addIfExtreme(edgeA->getTo(), postB, word, std::max(value, edgeA->getWeight()->getValue()))) {
						buffer->add(edgeA->getTo(), postB, word, std::max(value, edgeA->getWeight()->getValue()));
					}
				}

				if (postB->getRef() == 0) {
					delete word;
					delete postB;
				}
			}
		}
	}

	PostContextVariable* tmp;
	tmp = this->updates;
	this->updates = this->buffer;
	this->buffer = tmp;
	this->buffer->clear();

	return (this->updates->size() > 0);
}





