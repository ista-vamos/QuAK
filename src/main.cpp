#include <iostream>

#include "FORKLIFT/inclusion.h"
#include "Automaton.h"
#include "Map.h"
#include "Word.h"

int main(int argc, char **argv) {

	// Automaton* A = new Automaton("samples/A.txt");
	// std::cout << A->getTopValue(Inf) << std::endl;
	// delete A;

	Automaton* A = new Automaton("samples/test2.txt");
	A->print();
	Automaton* B = Automaton::determinizeInf(A);
	B->print();

	std::cout << A->isIncludedIn(B, Inf) << " " << B->isIncludedIn(A, Inf) << std::endl;

	delete A;
	delete B;

	// debug_test();

	return EXIT_SUCCESS;
}


