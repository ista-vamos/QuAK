#include <iostream>
#include "Automaton.h"


int main(int argc, char **argv) {
	printf("--------------------------------------------\n");
	Automaton* A = new Automaton("./samples/test2.txt");
	std::cout << std::endl << A->toString() << std::endl;
	printf("--------------------------------------------\n");
	Automaton* B = new Automaton("./samples/test3.txt");
	std::cout << std::endl << B->toString() << std::endl;
	printf("--------------------------------------------\n");
	Automaton* Ab = A->booleanize(1);
	std::cout << std::endl << Ab->toString() << std::endl;
	printf("--------------------------------------------\n");
	Automaton* Bb = B->booleanize(1);
	std::cout << std::endl << Bb->toString() << std::endl;
	printf("--------------------------------------------\n");

	// std::cout << "Deterministic(A): " << A->isDeterministic() << std::endl;
	// std::cout << "Deterministic(B): " << B->isDeterministic() << std::endl;
	// std::cout << "Empty(A,0): " << A->isEmpty(Sup, 0) << std::endl;
	// std::cout << "Empty(A,1000): " << A->isEmpty(Sup, 1000) << std::endl;
	// std::cout << "Included(A,B): " << A->isIncludedIn_det(Sup, B) << std::endl;
	// std::cout << "Included(B,A): " << B->isIncludedIn_det(Sup, A) << std::endl;
	// std::cout << "Equivalent(A,B): " << A->isIncludedIn_det(Sup, B) << std::endl;
	// std::cout << "Universal(A,0): " << A->isUniversal_det(Sup,0) << std::endl;
	// std::cout << "Universal(A,-100): " << A->isUniversal_det(Sup,-100) << std::endl;



	return EXIT_SUCCESS;
}
