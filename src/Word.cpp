#include "Word.h"
#include <vector>

Word::~Word() {
    if (all) {
        // First delete all Symbol objects
        for (Symbol* symbol : *all) {
            delete symbol;
        }
        // Then delete the vector itself
        delete all;
        all = nullptr;
    }
}

// Assignment operator with proper Word:: prefix
Word& Word::operator=(const Word& other) {
    if (this != &other) {
        // Clean up existing data
        for (Symbol* symbol : *all) {
            delete symbol;
        }
        all->clear();

        // Copy new data
        for (Symbol* symbol : *(other.all)) {
            all->push_back(new Symbol(*symbol));
        }
    }
    return *this;
}

Word::Word() {
    this->all = new std::vector<Symbol*>();
}

Word::Word(Symbol* symbol) {
    this->all = new std::vector<Symbol*>();
    this->all->push_back(new Symbol(*symbol)); // Create a copy
}

Word::Word(Word* word, Symbol* symbol) {
    this->all = new std::vector<Symbol*>();
    // Deep copy of existing symbols
    for (Symbol* s : *(word->all)) {
        this->all->push_back(new Symbol(*s));
    }
    // Copy the new symbol
    this->all->push_back(new Symbol(*symbol));
}

Word::Word(const Word& other) {
    this->all = new std::vector<Symbol*>();
    for (Symbol* symbol : *(other.all)) {
        this->all->push_back(new Symbol(*symbol));
    }
}

size_t Word::getLength() {
    return this->all->size();
}

Symbol* Word::at(unsigned int i) {
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
    this->all->push_back(new Symbol(*symbol)); // Create a copy
}

void Word::pop_back() {
    if (!all->empty()) {
        delete all->back(); // Delete the Symbol before removing it
        all->pop_back();
    }
}
