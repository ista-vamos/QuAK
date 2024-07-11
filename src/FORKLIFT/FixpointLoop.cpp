
#include "FixpointLoop.h"
#include "inclusion.h"

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
//				#ifdef INCLUSION_SCC_SEARCH_ACTIVE
//				if (edgeB->getFrom()->getTag() != edgeB->getTo()->getTag()) continue;
//				#endif
				init_setB->add(fromB, edgeB->getTo(), edgeB->getWeight()->getId());
			}
		}

		for (Edge* edgeA : *(initA->getSuccessors(symbol->getId()))) {
			#ifdef INCLUSION_SCC_SEARCH_ACTIVE
			if (edgeA->getFrom()->getTag() != edgeA->getTo()->getTag()) continue;
			#endif
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



bool FixpointLoop::apply () {
	auto iterA = this->updates->begin();
	for(; iterA != this->updates->end(); ++iterA) {
		auto iter_symbol = iterA->first->getAlphabet()->begin();
		for (; iter_symbol != iterA->first->getAlphabet()->end(); ++iter_symbol) {
			auto iterB = iterA->second->begin();
			for (; iterB != iterA->second->end(); ++iterB) {
				ContextOf* postB = new ContextOf(iterB->first, *iter_symbol);
				Word* word = new Word(iterB->second.first, *iter_symbol);
				weight_t value = iterB->second.second;

				for (Edge* edgeA : *(iterA->first->getSuccessors((*iter_symbol)->getId()))) {
					#ifdef INCLUSION_SCC_SEARCH_ACTIVE
					if (edgeA->getFrom()->getTag() != edgeA->getTo()->getTag()) continue;
					#endif
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





