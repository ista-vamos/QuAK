
#include "Word.h"
#include <vector>

Word::~Word () {
	delete this->all;
}

Word::Word () {
	this->all = new std::vector<Symbol*>();
}

Word::Word (Symbol* symbol) {
	this->all = new std::vector<Symbol*>();
	this->all->push_back(symbol);
}

Word::Word (Word* word, Symbol* symbol) {
	this->all = new std::vector<Symbol*>(word->all->begin(), word->all->end());
	this->all->push_back(symbol);
}


unsigned int Word::getLength () {
	return this->all->size();
}

Symbol* Word::at (unsigned int i) {
	return this->all->at(i);
}

std::string Word::toString() {
	std::string s = "";
	for (Symbol* symbol : *(this->all)) {
		s.append(symbol->toString());
	}
//	for (unsigned int i = 0; i < this->all->size(); ++i) {
//		s.append(this->all->at(i)->toString());
//	}
	return s;
}



