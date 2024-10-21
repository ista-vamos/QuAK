
#include "Word.h"
#include <vector>

Word::~Word () {
	delete this->all;
}
// Word::~Word() {
//     for (Symbol* symbol : *all) {
//         delete symbol;
//     }
//     delete all;
// }

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

Word::Word(const Word& other) {
    this->all = new std::vector<Symbol*>();
    for (const Symbol* symbol : *(other.all)) {
        this->all->push_back(new Symbol(*symbol));
    }
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
	return s;
}


void Word::push_back(Symbol* symbol) {
    this->all->push_back(symbol);
}
