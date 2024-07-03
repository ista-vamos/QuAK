#include <iostream>

#include "FORKLIFT/inclusion.h"
#include "Automaton.h"
#include "Map.h"
#include "Word.h"

int main(int argc, char **argv) {

	Automaton* A = new Automaton("samples/test5.txt");
	A->print();
	Automaton* B = Automaton::safetyClosure(A, LimAvg);
	B->print();

	// std::cout << A->isConstant(LimAvg) << std::endl;
	// std::cout << B->isConstant(LimAvg) << std::endl;
	std::cout << A->isSafe(LimAvg) << std::endl; //TODO
	// std::cout << A->isLive(LimAvg) << std::endl;
	
	delete A;
	delete B;

	// debug_test();

	return EXIT_SUCCESS;
}


