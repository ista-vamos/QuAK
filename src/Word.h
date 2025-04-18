
#ifndef WORD_H_
#define WORD_H_

#include <string>
#include <vector>
#include "Set.h"
#include "Symbol.h"

class Word {
private:
	std::vector<Symbol*>* all;
public:
	~Word();
	Word ();
	Word (Symbol* symbol);
	Word (Word* word, Symbol* symbol);
	Word(const Word& other);
	size_t getLength ();
	Symbol* at (unsigned int i);
	std::string toString();
	void push_back(Symbol* symbol);
	void pop_back();
	Word& operator=(const Word& other);

};

#endif /* WORD_H_ */


