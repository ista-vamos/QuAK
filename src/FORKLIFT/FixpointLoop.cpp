
#include "FixpointLoop.h"
#include "../utility.h"

FixpointLoop::~FixpointLoop () {
	this->buffer->clear();//fixme
	this->content->clear();//fixme
	this->updates->clear();//fixme

	this->nb_deleted += this->buffer->nb_debug;//fixme
	this->nb_deleted += this->content->nb_debug;//fixme
	this->nb_deleted += this->updates->nb_debug;//fixme

	if (this->nb_constructed != this->nb_deleted)
		fail("../memory leak");

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
		nb_constructed++;//fixme

		for (State* fromB : *initB) {
			for (Edge* edgeB : *(fromB->getSuccessors(symbol->getId()))) {
				// fixme: consider only edge within the current SCC (only if relevance applied)
				init_setB->add(fromB, edgeB->getTo(), edgeB->getWeight()->getId());
			}
		}

		for (Edge* edgeA : *(initA->getSuccessors(symbol->getId()))) {
			// fixme: consider only edge within the current SCC
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
	nb_constructed++;//fixme

	for (unsigned int weight_id = 0; weight_id < this->capacity; ++weight_id) {
		for (std::pair<State*,TargetOf*> pairB : *(currentB->at(weight_id))) {
			for (State* fromB: *(pairB.second)) {
				for (Edge* edgeB : *(fromB->getSuccessors(symbol->getId()))) {
					// fixme: consider only edge within the current SCC (only if relevance applied)
					weight_t max_weight_id = std::max(weight_id, edgeB->getWeight()->getId());
					postB->add(pairB.first, edgeB->getTo(), max_weight_id);
					// -- since weight are sorted
					// -- id_1 < id_2 <==>  value_1 < value_2
				}
			}
		}
	}
	return postB;
}




bool FixpointLoop::apply () {
	auto iterA = this->updates->begin();
	for(; iterA != this->updates->end(); ++iterA) {
		auto iter_symbol = iterA->first->getAlphabet()->begin();
		for (; iter_symbol != iterA->first->getAlphabet()->end(); ++iter_symbol) {
			auto iterB = iterA->second->begin();
			while (iterB != iterA->second->end()) {
				ContextOf* postB = post(iterB->first, *iter_symbol); //fixme:old
				//ContextOf* postB = new ContextOf(iterB->first, *iter_symbol);nb_constructed++;//fixme
				Word* word = new Word(iterB->second.first, *iter_symbol);
				weight_t value = iterB->second.second;
				++iterB;

				for (Edge* edgeA : *(iterA->first->getSuccessors((*iter_symbol)->getId()))) {
					// fixme: consider only edge within the current SCC
					if (addIfExtreme(edgeA->getTo(), postB, word, std::max(value, edgeA->getWeight()->getValue()))) {
						buffer->add(edgeA->getTo(), postB, word, std::max(value, edgeA->getWeight()->getValue()));
					}
				}

				if (postB->getRef() == 0) {
					delete word;
					delete postB;
					nb_deleted++;//fixme
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





