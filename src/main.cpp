#include <iostream>

#include "FORKLIFT/inclusion.h"
#include "Automaton.h"
#include "Map.h"
#include "Word.h"

int main(int argc, char **argv) {
	Automaton* A = new Automaton("samples/A.txt");
	A->print();
	std::cout << A->getTopValue(Inf) << std::endl;
	delete A;
	return EXIT_SUCCESS;
}


