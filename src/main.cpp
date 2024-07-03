#include <iostream>

#include "FORKLIFT/inclusion.h"
#include "Automaton.h"
#include "Map.h"
#include "Word.h"

int main(int argc, char **argv) {

	Automaton* A = new Automaton("samples/test6.txt");
	A->print();
	Automaton* B = Automaton::safetyClosure(A, Inf);
	B->print();

	std::cout << A->isConstant(Inf) << std::endl;
	std::cout << B->isConstant(Inf) << std::endl;
	std::cout << A->isSafe(Inf) << std::endl;
	std::cout << A->isLive(Inf) << std::endl;
	
	delete A;
	delete B;

	// debug_test();

	return EXIT_SUCCESS;
}


