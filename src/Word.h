
#ifndef WORD_H_
#define WORD_H_

#include <string>
#include <vector>
#include "Symbol.h"

class Word {
private:
	std::vector<Symbol*>* all;
public:
	~Word();
	Word ();
	Word (Symbol* symbol);
	Word (Word* word, Symbol* symbol);
	unsigned int getLength ();
	Symbol* at (unsigned int i);
	std::string toString();
};

#endif /* WORD_H_ */


