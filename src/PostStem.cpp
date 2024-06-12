
#include "PostStem.h"
#include "Word.h"


PostStem::PostStem (State* initA, State* initB, bool rev) {
	this->reversed_inclusion = rev;
	this->content = new PostTargetVariable();
	this->updates = new PostTargetVariable();
	this->buffer = new PostTargetVariable();

	TargetOf* target = new TargetOf();
	target->add(initB);
	this->updates->add(initA, target, new Word());
	this->content->add(initA, target, new Word());
}


bool PostStem::addIfExtreme (State* stateA, TargetOf* postB, Word* word) {
	if (this->reversed_inclusion)
		return this->content->addIfMax(stateA, postB, word);
	else
		return this->content->addIfMin(stateA, postB, word);
}


TargetOf* PostStem::post (TargetOf* current, Symbol* symbol) {
	TargetOf* postB = new TargetOf();
	for (auto iter = current->begin(); iter != current->end(); ++iter) {
		for (Edge* edge : *((*iter)->getSuccessors(symbol->getId())))  {
			postB->add(edge->getTo());
		}
	}
	return postB;
}


bool PostStem::apply () {
	for (std::pair<State*, SetStd<std::pair<TargetOf*,Word*>>*> fromA : *(this->updates)) {
		for (Symbol* symbol : *(fromA.first->getAlphabet())) {
			for (std::pair<TargetOf*,Word*> pair : *(fromA.second)) {
				TargetOf* postB = post(pair.first, symbol);
				Word* word = new Word(pair.second, symbol);
				bool flag = true;
				for (Edge* toA : *(fromA.first->getSuccessors(symbol->getId()))) {
					if(addIfExtreme(toA->getTo(), postB, word)) {
						buffer->add(toA->getTo(), postB, word);
						flag = false;
					}
				}
				if (flag == true) {
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

	return (updates->size() == 0);
}


