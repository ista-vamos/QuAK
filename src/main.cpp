#include <iostream>
#include "Automaton.h"


int main(int argc, char **argv) {
	printf("--------------------------------------------\n");

	Automaton A = Automaton("./samples/A.txt");
	std::cout << std::endl << A.toString() << std::endl;

	printf("--------------------------------------------\n");

	Automaton SupCloA = A.safetyClosure(Sup);
	std::cout << std::endl << SupCloA.toString() << std::endl;

	printf("--------------------------------------------\n");

	Automaton B = SupCloA.product(Sup, A, Minus);
	std::cout << std::endl << B.toString() << std::endl;

	printf("--------------------------------------------\n");

	// TODO: 
	// bug in the min/max weights of safety closure
	// bug in scc

	return EXIT_SUCCESS;
}
