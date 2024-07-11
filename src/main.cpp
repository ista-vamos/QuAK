#include <iostream>

#include "FORKLIFT/inclusion.h"
#include "Automaton.h"
#include "Map.h"
#include "Word.h"

int main(int argc, char **argv) {

	 Automaton* A = new Automaton("samples/C1.txt");
	 A->print();
	 delete A;
	 Automaton* B = new Automaton("samples/C2.txt");
	 B->print();
	 delete B;

//	Automaton* A = new Automaton("samples/All_Sturmian_words_contain_cubes_SUPERSET.txt");
//	A->print();
//	Automaton* B = Automaton::determinizeInf(A);
//	B->print();
//
//	std::cout << A->isIncludedIn(B, Inf) << " " << B->isIncludedIn(A, Inf) << std::endl;

	// delete A;
	// delete B;

	// debug_test2();

	return EXIT_SUCCESS;
}


