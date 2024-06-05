#include <iostream>
#include "Automaton.h"


int main(int argc, char **argv) {
	printf("--------------------------------------------\n");

	// I think this should be the usage. 
	// When a function returns an Automaton object (instead of a pointer), it doesn't behave well.
	// Maybe there is a way around it without using Automaton*, I don't know.
	Automaton* A = new Automaton("./samples/A.txt");
	std::cout << std::endl << A->toString() << std::endl;

	printf("--------------------------------------------\n");

	Automaton* SupCloA = A->safetyClosure(Sup)->trim();
	std::cout << std::endl << SupCloA->toString() << std::endl;

	printf("--------------------------------------------\n");

	Automaton* B = SupCloA->product(Sup, A, Minus)->trim();
	std::cout << std::endl << B->toString() << std::endl;

	printf("--------------------------------------------\n");

	Automaton* SupCloB = B->safetyClosure(Sup);
	std::cout << std::endl << SupCloB->toString() << std::endl;

	printf("--------------------------------------------\n");
	// TODO: 
	// bug in scc
	// check the basic algos

	return EXIT_SUCCESS;
}
