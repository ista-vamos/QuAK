#include <iostream>
#include <vector>

#include "FORKLIFT/inclusion.h"
#include "Automaton.h"
#include "Monitor.h"
#include "Map.h"
#include "Word.h"

int main(int argc, char **argv) {

	Automaton* A = new Automaton("samples/test3.txt");
	A->print();

	Monitor* M = new Monitor(A, Sup);

	std::vector<Symbol*> word;
	word.push_back(M->getAlphabet()->at(0));
	word.push_back(M->getAlphabet()->at(1));
	word.push_back(M->getAlphabet()->at(1));

	for (int i = 0; i < 3; i++) {
		std::cout << M->getLowest() << " " << M->getHighest() << std::endl;
		M->read(word[i]);
	}
	std::cout << M->getLowest() << " " << M->getHighest() << std::endl;

	delete A;

	return EXIT_SUCCESS;
}


